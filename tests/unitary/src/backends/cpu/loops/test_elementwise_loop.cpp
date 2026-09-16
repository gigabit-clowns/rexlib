// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/loops/elementwise_loop.hpp>
#include <backends/cpu/loops/element_index_tags.hpp>
#include <backends/cpu/loops/inner_loop_stride_dispatch.hpp>
#include <backends/cpu/loops/linear_index_run.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/loops/multidimensional_index.hpp>
#include <backends/cpu/plans/index_operands.hpp>

#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/layout/joint_layout_builder.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

// Exact tag type each stride was resolved to. `other` catches any tag outside
// the documented vocabulary (contiguous_stride_tag, broadcasting_stride_tag,
// std::ptrdiff_t), so a regression to a bare integral_constant is detected.
enum class stride_kind
{
	contiguous,
	broadcasting,
	runtime,
	other,
};

/**
 * @brief Description of a single stride tag received by the kernel.
 */
struct stride_info
{
	std::ptrdiff_t value;
	stride_kind kind;

	bool operator==(const stride_info &other) const noexcept
	{
		return value == other.value && kind == other.kind;
	}
};

// A runtime stride is forwarded as a plain std::ptrdiff_t.
std::ptrdiff_t stride_value(std::ptrdiff_t s) noexcept
{
	return s;
}

// Statically typed strides are integral_constant-like and expose a
// compile-time value.
template <typename T>
std::ptrdiff_t stride_value(T) noexcept
{
	return T::value;
}

// Classify by exact tag type. The non-template overloads match only the
// documented tags; anything else falls through to the template overload.
stride_kind stride_classify(contiguous_stride_tag) noexcept
{
	return stride_kind::contiguous;
}
stride_kind stride_classify(broadcasting_stride_tag) noexcept
{
	return stride_kind::broadcasting;
}
stride_kind stride_classify(std::ptrdiff_t) noexcept
{
	return stride_kind::runtime;
}
template <typename T>
stride_kind stride_classify(T) noexcept
{
	return stride_kind::other;
}

/**
 * @brief Record of a single vector-loop kernel invocation.
 */
struct vector_loop_call
{
	std::vector<const int*> pointers;
	std::size_t count;
	std::vector<stride_info> strides;
};

/**
 * @brief Kernel that records every invocation.
 *
 * run_elementwise_vector_loop invokes the kernel as `(pointers, strides,
 * count)`, i.e. a tuple of `n` operand pointers, a tuple of `n` resolved
 * strides and the element count. The two tuples are unpacked and stored for
 * inspection. The log is held through a shared_ptr so that the copy stored
 * inside the loop shares state with the handle kept by the test.
 */
class recording_vector_kernel
{
public:
	recording_vector_kernel()
		: m_calls(std::make_shared<std::vector<vector_loop_call>>())
	{
	}

	template <typename PointerTuple, typename StrideTuple>
	void operator()(
		const PointerTuple &pointers,
		const StrideTuple &strides,
		std::size_t count
	) const
	{
		REXLIB_CONST_CONSTEXPR std::size_t n =
			std::tuple_size<PointerTuple>::value;

		vector_loop_call call;
		collect_pointers(call.pointers, pointers, std::make_index_sequence<n>());
		call.count = count;
		collect_strides(call.strides, strides, std::make_index_sequence<n>());
		m_calls->push_back(std::move(call));
	}

	const std::vector<vector_loop_call>& calls() const noexcept
	{
		return *m_calls;
	}

private:
	std::shared_ptr<std::vector<vector_loop_call>> m_calls;

	template <typename Tuple, std::size_t... Is>
	static void collect_pointers(
		std::vector<const int*> &out,
		const Tuple &pointers,
		std::index_sequence<Is...>
	)
	{
		(void) std::initializer_list<int> {
			(out.push_back(std::get<Is>(pointers)), 0)...
		};
	}

	template <typename Tuple, std::size_t... Is>
	static void collect_strides(
		std::vector<stride_info> &out,
		const Tuple &strides,
		std::index_sequence<Is...>
	)
	{
		(void) std::initializer_list<int> {
			(
				out.push_back(
					stride_info {
						stride_value(std::get<Is>(strides)),
						stride_classify(std::get<Is>(strides))
					}
				),
				0
			)...
		};
	}
};

/**
 * @brief Record of a single inner-loop invocation.
 */
struct outer_loop_call
{
	std::vector<const int*> pointers;
	std::size_t count;
};

/**
 * @brief Inner-loop functor that records every invocation.
 *
 * run_elementwise_outer_loop invokes the handler as (pointers..., count). The
 * count is the last argument, so it is captured together with the pointers and
 * split out afterwards (a trailing std::size_t after a pack is a non-deduced
 * context). The recorded log is held through a shared_ptr so that the copy
 * stored inside the outer loop shares state with the handle kept by the test.
 */
class recording_inner_loop
{
public:
	recording_inner_loop()
		: m_calls(std::make_shared<std::vector<outer_loop_call>>())
	{
	}

	template <typename... Args>
	void operator()(Args... args) const
	{
		constexpr std::size_t count_index = sizeof...(Args) - 1;
		const auto packed = std::make_tuple(args...);

		outer_loop_call call;
		call.count = static_cast<std::size_t>(std::get<count_index>(packed));
		collect_pointers(
			call.pointers,
			packed,
			std::make_index_sequence<count_index>()
		);
		m_calls->push_back(std::move(call));
	}

	const std::vector<outer_loop_call>& calls() const noexcept
	{
		return *m_calls;
	}

private:
	std::shared_ptr<std::vector<outer_loop_call>> m_calls;

	template <typename Tuple, std::size_t... Is>
	static void collect_pointers(
		std::vector<const int*> &out,
		const Tuple &packed,
		std::index_sequence<Is...>
	)
	{
		(void) std::initializer_list<int> {
			(out.push_back(std::get<Is>(packed)), 0)...
		};
	}
};

/**
 * @brief Description of one operand for make_layout.
 */
struct operand_spec
{
	std::vector<std::ptrdiff_t> strides;
	std::ptrdiff_t offset;
};

/**
 * @brief Build a layout with the provided iteration extents and operands.
 *
 * Optimizations are disabled so that the axis order (dim 0 being the inner
 * loop) and the strides are preserved verbatim.
 */
joint_layout make_layout(
	const std::vector<std::size_t> &extents,
	const std::vector<operand_spec> &operands
)
{
	joint_layout_builder builder;
	builder.set_extents(make_span(extents));
	for (const auto &operand : operands)
	{
		builder.add_operand(
			make_span(extents),
			make_span(operand.strides),
			operand.offset
		);
	}
	return builder.build({});
}

// Backing storage the operand pointers point into. Large enough that all the
// offsets produced by the tests stay in bounds.
std::vector<int> make_buffer()
{
	return std::vector<int>(2048);
}

// Offsets of a vector-loop call's pointers relative to the common base
// pointer.
std::vector<std::ptrdiff_t> offsets_of(
	const vector_loop_call &call,
	const int *base
)
{
	std::vector<std::ptrdiff_t> result;
	for (const auto *pointer : call.pointers)
	{
		result.push_back(pointer - base);
	}
	return result;
}

// Offsets of an outer-loop call's pointers relative to the common base
// pointer.
std::vector<std::ptrdiff_t> offsets_of(
	const outer_loop_call &call,
	const int *base
)
{
	std::vector<std::ptrdiff_t> result;
	for (const auto *pointer : call.pointers)
	{
		result.push_back(pointer - base);
	}
	return result;
}

} // namespace


TEST_CASE(
	"elementwise_outer_loop invokes the handler once for a 1D layout",
	"[elementwise_outer_loop]"
)
{
	auto buffer = make_buffer();
	recording_inner_loop recorder;
	run_elementwise_outer_loop(
		recorder,
		make_layout({5}, {{{1}, 0}}),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 1 );
	CHECK( calls[0].count == 5 );
	CHECK( offsets_of(calls[0], buffer.data()) == std::vector<std::ptrdiff_t>{0} );
}

TEST_CASE(
	"elementwise_outer_loop iterates over the outer dimension of a 2D "
	"layout",
	"[elementwise_outer_loop]"
)
{
	auto buffer = make_buffer();
	recording_inner_loop recorder;
	// Inner extent 3 (dim 0), outer extent 4 (dim 1) with outer stride 100.
	run_elementwise_outer_loop(
		recorder,
		make_layout({3, 4}, {{{1, 100}, 0}}),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 4 );
	const std::vector<std::ptrdiff_t> expected_offsets = {0, 100, 200, 300};
	for (std::size_t i = 0; i < calls.size(); ++i)
	{
		CHECK( calls[i].count == 3 );
		CHECK(
			offsets_of(calls[i], buffer.data()) ==
			std::vector<std::ptrdiff_t>{expected_offsets[i]}
		);
	}
}

TEST_CASE(
	"elementwise_outer_loop varies the innermost outer dimension fastest",
	"[elementwise_outer_loop]"
)
{
	auto buffer = make_buffer();
	recording_inner_loop recorder;
	// Extents {2, 2, 2}: dim 1 (stride 10) varies before dim 2 (stride 100).
	run_elementwise_outer_loop(
		recorder,
		make_layout({2, 2, 2}, {{{1, 10, 100}, 0}}),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 4 );
	const std::vector<std::ptrdiff_t> expected_offsets = {0, 10, 100, 110};
	for (std::size_t i = 0; i < calls.size(); ++i)
	{
		CHECK( calls[i].count == 2 );
		CHECK(
			offsets_of(calls[i], buffer.data()) ==
			std::vector<std::ptrdiff_t>{expected_offsets[i]}
		);
	}
}

TEST_CASE(
	"elementwise_outer_loop advances each operand by its own stride and "
	"base offset",
	"[elementwise_outer_loop]"
)
{
	auto buffer = make_buffer();
	recording_inner_loop recorder;
	// Two operands sharing extents {2, 3}. The second one has a base offset.
	run_elementwise_outer_loop(
		recorder,
		make_layout(
			{2, 3},
			{
				{{1, 10}, 0},
				{{1, 20}, 1000}
			}
		),
		buffer.data(),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 3 );
	const std::vector<std::vector<std::ptrdiff_t>> expected = {
		{0, 1000},
		{10, 1020},
		{20, 1040}
	};
	for (std::size_t i = 0; i < calls.size(); ++i)
	{
		CHECK( calls[i].count == 2 );
		CHECK( offsets_of(calls[i], buffer.data()) == expected[i] );
	}
}

TEST_CASE(
	"elementwise_outer_loop invokes the handler once with count 1 for a "
	"rank-0 layout",
	"[elementwise_outer_loop]"
)
{
	auto buffer = make_buffer();
	recording_inner_loop recorder;
	// Empty extents, single operand with a base offset.
	run_elementwise_outer_loop(
		recorder,
		make_layout({}, {{{}, 7}}),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 1 );
	CHECK( calls[0].count == 1 );
	CHECK( offsets_of(calls[0], buffer.data()) == std::vector<std::ptrdiff_t>{7} );
}

TEST_CASE(
	"elementwise_outer_loop does not invoke the handler for an empty axis",
	"[elementwise_outer_loop]"
)
{
	auto buffer = make_buffer();
	recording_inner_loop recorder;
	run_elementwise_outer_loop(
		recorder,
		make_layout({0}, {{{1}, 0}}),
		buffer.data()
	);

	CHECK( recorder.calls().empty() );
}

TEST_CASE(
	"run_elementwise_outer_loop accepts an rvalue inner loop",
	"[elementwise_outer_loop]"
)
{
	auto buffer = make_buffer();
	recording_inner_loop recorder;
	// Pass a temporary inner loop (sharing the recorder's log) to exercise the
	// forwarding reference.
	run_elementwise_outer_loop(
		recording_inner_loop(recorder),
		make_layout({3}, {{{1}, 5}}),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 1 );
	CHECK( calls[0].count == 3 );
	CHECK( offsets_of(calls[0], buffer.data()) == std::vector<std::ptrdiff_t>{5} );
}


TEST_CASE(
	"run_elementwise_vector_loop invokes the kernel once for a contiguous 1D "
	"layout",
	"[elementwise_vector_loop]"
)
{
	auto buffer = make_buffer();
	recording_vector_kernel recorder;
	run_elementwise_vector_loop(
		recorder,
		make_layout({5}, {{{1}, 0}}),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 1 );
	CHECK( calls[0].count == 5 );
	CHECK(
		offsets_of(calls[0], buffer.data()) == std::vector<std::ptrdiff_t>{0}
	);
	REQUIRE( calls[0].strides.size() == 1 );
	CHECK( calls[0].strides[0] == stride_info{1, stride_kind::contiguous} );
}

TEST_CASE(
	"run_elementwise_vector_loop resolves each operand's inner stride "
	"independently",
	"[elementwise_vector_loop]"
)
{
	auto buffer = make_buffer();
	recording_vector_kernel recorder;
	// Three operands sharing extents {4} with contiguous, broadcasting and
	// generic inner strides respectively.
	run_elementwise_vector_loop(
		recorder,
		make_layout(
			{4},
			{
				{{1}, 0},
				{{0}, 1000},
				{{7}, 500}
			}
		),
		buffer.data(),
		buffer.data(),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 1 );
	CHECK( calls[0].count == 4 );
	const std::vector<stride_info> expected_strides = {
		{1, stride_kind::contiguous},
		{0, stride_kind::broadcasting},
		{7, stride_kind::runtime}
	};
	CHECK( calls[0].strides == expected_strides );
}

TEST_CASE(
	"run_elementwise_vector_loop iterates the outer dimensions and passes "
	"the stride tags on every call",
	"[elementwise_vector_loop]"
)
{
	auto buffer = make_buffer();
	recording_vector_kernel recorder;
	// Inner extent 3 (dim 0, stride 1), outer extent 4 (dim 1, stride 100).
	run_elementwise_vector_loop(
		recorder,
		make_layout({3, 4}, {{{1, 100}, 0}}),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 4 );
	const std::vector<std::ptrdiff_t> expected_offsets = {0, 100, 200, 300};
	for (std::size_t i = 0; i < calls.size(); ++i)
	{
		CHECK( calls[i].count == 3 );
		CHECK(
			offsets_of(calls[i], buffer.data()) ==
			std::vector<std::ptrdiff_t>{expected_offsets[i]}
		);
		REQUIRE( calls[i].strides.size() == 1 );
		CHECK( calls[i].strides[0] == stride_info{1, stride_kind::contiguous} );
	}
}

TEST_CASE(
	"run_elementwise_vector_loop advances each operand by its own stride "
	"and base offset",
	"[elementwise_vector_loop]"
)
{
	auto buffer = make_buffer();
	recording_vector_kernel recorder;
	// Two operands sharing extents {2, 3}; the second one has a base offset.
	run_elementwise_vector_loop(
		recorder,
		make_layout(
			{2, 3},
			{
				{{1, 10}, 0},
				{{1, 20}, 1000}
			}
		),
		buffer.data(),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 3 );
	const std::vector<std::vector<std::ptrdiff_t>> expected = {
		{0, 1000},
		{10, 1020},
		{20, 1040}
	};
	for (std::size_t i = 0; i < calls.size(); ++i)
	{
		CHECK( calls[i].count == 2 );
		CHECK( offsets_of(calls[i], buffer.data()) == expected[i] );
		const std::vector<stride_info> expected_strides = {
			{1, stride_kind::contiguous},
			{1, stride_kind::contiguous}
		};
		CHECK( calls[i].strides == expected_strides );
	}
}

TEST_CASE(
	"run_elementwise_vector_loop invokes the kernel once with count 1 and "
	"broadcasting tags for a rank-0 layout",
	"[elementwise_vector_loop]"
)
{
	auto buffer = make_buffer();
	recording_vector_kernel recorder;
	// Empty extents: a single element and no stride to inspect, so the operand
	// collapses to a broadcasting tag.
	run_elementwise_vector_loop(
		recorder,
		make_layout({}, {{{}, 7}}),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 1 );
	CHECK( calls[0].count == 1 );
	CHECK(
		offsets_of(calls[0], buffer.data()) == std::vector<std::ptrdiff_t>{7}
	);
	REQUIRE( calls[0].strides.size() == 1 );
	CHECK( calls[0].strides[0] == stride_info{0, stride_kind::broadcasting} );
}

TEST_CASE(
	"run_elementwise_vector_loop does not invoke the kernel for an empty "
	"axis",
	"[elementwise_vector_loop]"
)
{
	auto buffer = make_buffer();
	recording_vector_kernel recorder;
	run_elementwise_vector_loop(
		recorder,
		make_layout({0}, {{{1}, 0}}),
		buffer.data()
	);

	CHECK( recorder.calls().empty() );
}

TEST_CASE(
	"run_elementwise_vector_loop accepts an rvalue kernel",
	"[elementwise_vector_loop]"
)
{
	auto buffer = make_buffer();
	recording_vector_kernel recorder;
	// Pass a temporary kernel (sharing the recorder's log) to exercise the
	// forwarding reference.
	run_elementwise_vector_loop(
		recording_vector_kernel(recorder),
		make_layout({3}, {{{1}, 5}}),
		buffer.data()
	);

	const auto &calls = recorder.calls();
	REQUIRE( calls.size() == 1 );
	CHECK( calls[0].count == 3 );
	CHECK(
		offsets_of(calls[0], buffer.data()) == std::vector<std::ptrdiff_t>{5}
	);
	REQUIRE( calls[0].strides.size() == 1 );
	CHECK( calls[0].strides[0] == stride_info{1, stride_kind::contiguous} );
}


TEST_CASE(
	"run_elementwise_loop applies the operation to every element of a "
	"contiguous 1D layout",
	"[elementwise_loop]"
)
{
	std::vector<int> result(4, 0);
	std::vector<int> x = {1, 2, 3, 4};
	std::vector<int> y = {10, 20, 30, 40};

	run_elementwise_loop(
		[] (int *r, const int *a, const int *b) { *r = *a + *b; },
		make_layout({4}, {{{1}, 0}, {{1}, 0}, {{1}, 0}}),
		result.data(),
		x.data(),
		y.data()
	);

	CHECK( result == std::vector<int>{11, 22, 33, 44} );
}

TEST_CASE(
	"run_elementwise_loop applies a unary operation over a single operand",
	"[elementwise_loop]"
)
{
	std::vector<int> result(3, 0);
	std::vector<int> x = {1, -2, 3};

	run_elementwise_loop(
		[] (int *r, const int *a) { *r = -(*a); },
		make_layout({3}, {{{1}, 0}, {{1}, 0}}),
		result.data(),
		x.data()
	);

	CHECK( result == std::vector<int>{-1, 2, -3} );
}

TEST_CASE(
	"run_elementwise_loop holds a broadcasted operand fixed across every "
	"element",
	"[elementwise_loop]"
)
{
	std::vector<int> result(3, 0);
	std::vector<int> x = {1, 2, 3};
	int y_scalar = 100;

	run_elementwise_loop(
		[] (int *r, const int *a, const int *b) { *r = *a + *b; },
		make_layout({3}, {{{1}, 0}, {{1}, 0}, {{0}, 0}}),
		result.data(),
		x.data(),
		&y_scalar
	);

	CHECK( result == std::vector<int>{101, 102, 103} );
}

TEST_CASE(
	"run_elementwise_loop applies the operation through a runtime (non-unit) "
	"stride",
	"[elementwise_loop]"
)
{
	// result and x are packed with stride 2; y is contiguous. The odd slots of
	// result and x are sentinels that must not be touched.
	std::vector<int> result(7, -1);
	std::vector<int> x(7, 0);
	for (std::size_t i = 0; i < 4; ++i)
	{
		x[i * 2] = static_cast<int>(i + 1);
	}
	std::vector<int> y = {100, 200, 300, 400};

	run_elementwise_loop(
		[] (int *r, const int *a, const int *b) { *r = *a + *b; },
		make_layout({4}, {{{2}, 0}, {{2}, 0}, {{1}, 0}}),
		result.data(),
		x.data(),
		y.data()
	);

	CHECK( result[0] == 101 );
	CHECK( result[2] == 202 );
	CHECK( result[4] == 303 );
	CHECK( result[6] == 404 );
	CHECK( result[1] == -1 );
	CHECK( result[3] == -1 );
	CHECK( result[5] == -1 );
}

TEST_CASE(
	"run_elementwise_loop applies the operation across every outer "
	"dimension of a multidimensional layout",
	"[elementwise_loop]"
)
{
	std::vector<int> result(400, -1);
	std::vector<int> x(400, 0);
	std::vector<int> y(400, 0);
	// Four inner vectors of 3 elements each, spaced 100 apart.
	for (std::size_t outer = 0; outer < 4; ++outer)
	{
		for (std::size_t inner = 0; inner < 3; ++inner)
		{
			const auto index = outer * 100 + inner;
			x[index] = static_cast<int>(outer * 10 + inner);
			y[index] = static_cast<int>(outer);
		}
	}

	run_elementwise_loop(
		[] (int *r, const int *a, const int *b) { *r = *a + *b; },
		make_layout(
			{3, 4},
			{
				{{1, 100}, 0},
				{{1, 100}, 0},
				{{1, 100}, 0}
			}
		),
		result.data(),
		x.data(),
		y.data()
	);

	for (std::size_t outer = 0; outer < 4; ++outer)
	{
		for (std::size_t inner = 0; inner < 3; ++inner)
		{
			const auto index = outer * 100 + inner;
			CHECK(
				result[index] ==
				static_cast<int>(outer * 10 + inner + outer)
			);
		}
	}
}

TEST_CASE(
	"run_elementwise_loop supports operations with more than one output "
	"operand",
	"[elementwise_loop]"
)
{
	// No input/output segregation: an op with two outputs and two inputs is
	// expressed the same way as a unary or binary one.
	std::vector<int> quotient(3, 0);
	std::vector<int> remainder(3, 0);
	std::vector<int> a = {17, 20, 9};
	std::vector<int> b = {5, 3, 2};

	run_elementwise_loop(
		[] (int *q, int *r, const int *x, const int *y)
		{
			*q = *x / *y;
			*r = *x % *y;
		},
		make_layout({3}, {{{1}, 0}, {{1}, 0}, {{1}, 0}, {{1}, 0}}),
		quotient.data(),
		remainder.data(),
		a.data(),
		b.data()
	);

	CHECK( quotient == std::vector<int>{3, 6, 4} );
	CHECK( remainder == std::vector<int>{2, 2, 1} );
}

TEST_CASE(
	"run_elementwise_loop does not invoke the operation for an empty axis",
	"[elementwise_loop]"
)
{
	std::size_t calls = 0;
	int dummy = 0;

	run_elementwise_loop(
		[&calls] (int *r) { ++calls; *r = 1; },
		make_layout({0}, {{{1}, 0}}),
		&dummy
	);

	CHECK( calls == 0 );
}

namespace
{

/**
 * @brief Every element an outer loop visits, in the order it visits them.
 *
 * Recorded as the offset of each element from the operand's base pointer, so
 * that a walk over a range and a walk over the whole space can be compared
 * without either knowing which vector it was in the middle of.
 */
std::vector<std::ptrdiff_t> visit_range(
	const joint_layout &layout,
	std::size_t begin,
	std::size_t end,
	std::ptrdiff_t stride
)
{
	std::vector<std::ptrdiff_t> result;

	// A real buffer rather than a null base: the offsets are only ever read
	// back as differences, but forming them from a null pointer would not be
	// pointer arithmetic the language defines.
	auto buffer = make_buffer();
	int *const base = buffer.data();

	run_elementwise_outer_loop_range(
		[&result, base, stride] (int *pointer, std::size_t count)
		{
			for (std::size_t i = 0; i < count; ++i)
			{
				result.push_back(
					(pointer - base) + static_cast<std::ptrdiff_t>(i)*stride
				);
			}
		},
		layout,
		begin,
		end,
		base
	);

	return result;
}

} // anonymous namespace

TEST_CASE(
	"run_elementwise_outer_loop_range should visit between two calls exactly "
	"what one call visits",
	"[elementwise_loop]"
)
{
	// Brute force over every way of cutting the iteration space in two. The
	// whole point of the ranged walk is that the cut is invisible in what
	// gets visited, so every cut is worth checking, including the ones that
	// fall in the middle of a vector.
	std::vector<std::size_t> extents;
	std::vector<operand_spec> operands;
	std::ptrdiff_t inner_stride = 1;

	SECTION( "a contiguous vector" )
	{
		extents = { 8 };
		operands = { { { 1 }, 0 } };
	}
	SECTION( "several vectors" )
	{
		extents = { 4, 3 };
		operands = { { { 1, 4 }, 0 } };
	}
	SECTION( "an operand walked with a stride" )
	{
		extents = { 4, 3 };
		operands = { { { 3, 40 }, 7 } };
		inner_stride = 3;
	}
	SECTION( "an operand broadcast along the inner axis" )
	{
		extents = { 4, 3 };
		operands = { { { 0, 4 }, 0 } };
		inner_stride = 0;
	}
	SECTION( "a deeper space" )
	{
		extents = { 2, 3, 2 };
		operands = { { { 1, 2, 6 }, 0 } };
	}
	SECTION( "an axis of extent one" )
	{
		extents = { 3, 1, 2 };
		operands = { { { 1, 3, 3 }, 0 } };
	}

	const auto layout = make_layout(extents, operands);
	const auto count = layout.compute_element_count();

	const auto whole = visit_range(layout, 0, count, inner_stride);
	REQUIRE( whole.size() == count );

	for (std::size_t cut = 0; cut <= count; ++cut)
	{
		auto joined = visit_range(layout, 0, cut, inner_stride);
		const auto tail = visit_range(layout, cut, count, inner_stride);
		joined.insert(joined.end(), tail.cbegin(), tail.cend());

		INFO( "cut at " << cut );
		CHECK( joined == whole );
	}
}

TEST_CASE(
	"run_elementwise_outer_loop_range should visit nothing for an empty range",
	"[elementwise_loop]"
)
{
	const auto layout = make_layout({ 4, 3 }, { { { 1, 4 }, 0 } });

	CHECK( visit_range(layout, 0, 0, 1).empty() );
	CHECK( visit_range(layout, 5, 5, 1).empty() );
}

TEST_CASE(
	"run_elementwise_outer_loop_range should visit nothing for an empty axis",
	"[elementwise_loop]"
)
{
	const auto layout = make_layout({ 4, 0 }, { { { 1, 4 }, 0 } });

	CHECK( visit_range(layout, 0, 0, 1).empty() );
}

TEST_CASE(
	"run_elementwise_outer_loop should visit the whole space of a rank zero "
	"layout once",
	"[elementwise_loop]"
)
{
	// A layout of no axes holds one position, and the loop has to visit it:
	// this is the case where asking the cursor for a remainder would answer
	// zero and skip it.
	const auto layout = make_layout({}, { { {}, 5 } });

	CHECK( visit_range(layout, 0, 1, 1) ==
	       std::vector<std::ptrdiff_t>{ 5 } );
}

namespace
{

/**
 * @brief Build a layout over array operands, followed by the index operands
 * over the axes of those arrays.
 *
 * The axes are listed as an array lists them, and the builder is left free to
 * reorder and coalesce them.
 */
template <typename Indexing>
joint_layout make_indexed_layout(
	const std::vector<std::size_t> &extents,
	const std::vector<operand_spec> &operands,
	Indexing indexing
)
{
	joint_layout_builder builder;
	builder.set_extents(make_span(extents));
	for (const auto &operand : operands)
	{
		builder.add_operand(
			make_span(extents),
			make_span(operand.strides),
			operand.offset
		);
	}
	add_index_operands(builder, make_span(extents), indexing);
	return builder.build();
}

struct linear_index_writer
{
	void operator()(std::ptrdiff_t *destination, std::size_t index) const
	{
		*destination = static_cast<std::ptrdiff_t>(index);
	}
};

/**
 * @brief Writes the coordinates of an element as the digits of a base 100
 * number, the first axis being the most significant.
 */
struct coordinate_writer
{
	void operator()(
		std::ptrdiff_t *destination,
		const multidimensional_index &index
	) const
	{
		std::ptrdiff_t value = 0;
		for (std::size_t axis = 0; axis < index.get_rank(); ++axis)
		{
			value = value*100 + static_cast<std::ptrdiff_t>(index[axis]);
		}
		*destination = value;
	}
};

struct linear_vector_call
{
	std::size_t count;
	std::size_t first_index;
	std::size_t second_index;
	bool contiguous_stride;

	bool operator==(const linear_vector_call &other) const noexcept
	{
		return
			count == other.count &&
			first_index == other.first_index &&
			second_index == other.second_index &&
			contiguous_stride == other.contiguous_stride;
	}
};

/**
 * @brief Vector kernel recording where the linear index run of every vector
 * starts, where it is one element later, and how its stride was resolved.
 */
class linear_vector_recorder
{
public:
	explicit linear_vector_recorder(std::vector<linear_vector_call> &calls)
		: m_calls(&calls)
	{
	}

	template <typename Pointers, typename Strides, typename Stride>
	void operator()(
		const Pointers& /*pointers*/,
		const Strides& /*strides*/,
		std::size_t count,
		const linear_index_run<Stride> &index_run
	) const
	{
		m_calls->push_back(
			linear_vector_call {
				count,
				index_run.get_index(),
				index_run.advanced(1).get_index(),
				std::is_same<Stride, contiguous_stride_tag>::value
			}
		);
	}

private:
	std::vector<linear_vector_call> *m_calls;
};

} // anonymous namespace

TEST_CASE(
	"run_indexed_elementwise_loop should hand every element of a contiguous "
	"operand its linear index",
	"[indexed_elementwise_loop]"
)
{
	std::vector<std::ptrdiff_t> destination(5, -1);

	run_indexed_elementwise_loop(
		linear_index_writer(),
		make_indexed_layout({ 5 }, { { { 1 }, 0 } }, linear_index_tag()),
		linear_index_tag(),
		destination.data()
	);

	CHECK( destination == std::vector<std::ptrdiff_t>{ 0, 1, 2, 3, 4 } );
}

TEST_CASE(
	"run_indexed_elementwise_loop should count the linear index from the "
	"first element whatever the stride",
	"[indexed_elementwise_loop]"
)
{
	// Every third element is written, and the ones in between are left alone.
	std::vector<std::ptrdiff_t> destination(7, -1);

	run_indexed_elementwise_loop(
		linear_index_writer(),
		make_indexed_layout({ 3 }, { { { 3 }, 0 } }, linear_index_tag()),
		linear_index_tag(),
		destination.data()
	);

	CHECK( destination ==
	       std::vector<std::ptrdiff_t>{ 0, -1, -1, 1, -1, -1, 2 } );
}

TEST_CASE(
	"run_indexed_elementwise_loop should count the linear index backwards "
	"through memory on a negative stride",
	"[indexed_elementwise_loop]"
)
{
	std::vector<std::ptrdiff_t> destination(4, -1);

	run_indexed_elementwise_loop(
		linear_index_writer(),
		make_indexed_layout({ 4 }, { { { -1 }, 3 } }, linear_index_tag()),
		linear_index_tag(),
		destination.data()
	);

	CHECK( destination == std::vector<std::ptrdiff_t>{ 3, 2, 1, 0 } );
}

TEST_CASE(
	"run_indexed_elementwise_loop should write nothing to an empty operand",
	"[indexed_elementwise_loop]"
)
{
	std::vector<std::ptrdiff_t> destination(3, -1);

	run_indexed_elementwise_loop(
		linear_index_writer(),
		make_indexed_layout({ 0 }, { { { 1 }, 0 } }, linear_index_tag()),
		linear_index_tag(),
		destination.data()
	);

	CHECK( destination == std::vector<std::ptrdiff_t>{ -1, -1, -1 } );
}

TEST_CASE(
	"run_indexed_elementwise_loop should hand out every linear index exactly "
	"once",
	"[indexed_elementwise_loop]"
)
{
	std::vector<std::size_t> visits;
	const auto recorder =
		[&visits] (std::ptrdiff_t *destination, std::size_t index)
		{
			visits.push_back(index);
			*destination = 0;
		};

	std::vector<std::ptrdiff_t> destination(4, -1);
	run_indexed_elementwise_loop(
		recorder,
		make_indexed_layout({ 4 }, { { { 1 }, 0 } }, linear_index_tag()),
		linear_index_tag(),
		destination.data()
	);

	CHECK( visits == std::vector<std::size_t>{ 0, 1, 2, 3 } );
}

TEST_CASE(
	"run_indexed_elementwise_loop should count the linear index in row-major "
	"order over a column-major operand",
	"[indexed_elementwise_loop]"
)
{
	// Element (i, j) of a 2x3 array stored column by column sits at i + 2j,
	// and its linear index is 3i + j.
	std::vector<std::ptrdiff_t> destination(6, -1);

	run_indexed_elementwise_loop(
		linear_index_writer(),
		make_indexed_layout({ 2, 3 }, { { { 1, 2 }, 0 } }, linear_index_tag()),
		linear_index_tag(),
		destination.data()
	);

	CHECK( destination == std::vector<std::ptrdiff_t>{ 0, 3, 1, 4, 2, 5 } );
}

TEST_CASE(
	"run_indexed_elementwise_loop should hand the index after the pointer of "
	"every operand",
	"[indexed_elementwise_loop]"
)
{
	std::vector<std::ptrdiff_t> destination(6, -1);
	const std::vector<std::ptrdiff_t> row = { 10, 20, 30 };

	run_indexed_elementwise_loop(
		[] (
			std::ptrdiff_t *result,
			const std::ptrdiff_t *value,
			std::size_t index
		)
		{
			*result = *value + static_cast<std::ptrdiff_t>(index);
		},
		make_indexed_layout(
			{ 2, 3 },
			{ { { 3, 1 }, 0 }, { { 0, 1 }, 0 } },
			linear_index_tag()
		),
		linear_index_tag(),
		destination.data(),
		row.data()
	);

	CHECK( destination ==
	       std::vector<std::ptrdiff_t>{ 10, 21, 32, 13, 24, 35 } );
}

TEST_CASE(
	"run_indexed_elementwise_loop should hand every element of a column-major "
	"operand its coordinates",
	"[indexed_elementwise_loop]"
)
{
	// Element (i, j, k) of a 2x3x4 array stored column by column sits at
	// i + 2j + 6k.
	std::vector<std::ptrdiff_t> destination(24, -1);

	run_indexed_elementwise_loop(
		coordinate_writer(),
		make_indexed_layout(
			{ 2, 3, 4 },
			{ { { 1, 2, 6 }, 0 } },
			multidimensional_index_tag()
		),
		multidimensional_index_tag(),
		destination.data()
	);

	for (std::size_t i = 0; i < 2; ++i)
	{
		for (std::size_t j = 0; j < 3; ++j)
		{
			for (std::size_t k = 0; k < 4; ++k)
			{
				CHECK(
					destination[i + 2*j + 6*k] ==
					static_cast<std::ptrdiff_t>(10000*i + 100*j + k)
				);
			}
		}
	}
}

TEST_CASE(
	"run_indexed_elementwise_loop should hand a coordinate of zero along an "
	"axis of extent one",
	"[indexed_elementwise_loop]"
)
{
	// Element (i, 0, k) of a 3x1x2 row-major array sits at 2i + k.
	std::vector<std::ptrdiff_t> destination(6, -1);

	run_indexed_elementwise_loop(
		coordinate_writer(),
		make_indexed_layout(
			{ 3, 1, 2 },
			{ { { 2, 2, 1 }, 0 } },
			multidimensional_index_tag()
		),
		multidimensional_index_tag(),
		destination.data()
	);

	CHECK( destination ==
	       std::vector<std::ptrdiff_t>{ 0, 1, 10000, 10001, 20000, 20001 } );
}

TEST_CASE(
	"run_indexed_elementwise_loop should hand the element of a rank zero "
	"operand coordinates of rank zero",
	"[indexed_elementwise_loop]"
)
{
	std::size_t rank = 1;
	std::ptrdiff_t destination = -1;

	run_indexed_elementwise_loop(
		[&rank] (std::ptrdiff_t *result, const multidimensional_index &index)
		{
			rank = index.get_rank();
			*result = 7;
		},
		make_indexed_layout({}, { { {}, 0 } }, multidimensional_index_tag()),
		multidimensional_index_tag(),
		&destination
	);

	CHECK( rank == 0 );
	CHECK( destination == 7 );
}

TEST_CASE(
	"run_indexed_elementwise_loop should hand out the same indices however "
	"many threads run it",
	"[indexed_elementwise_loop]"
)
{
	// Column-major, so that the traversal does not visit the elements in the
	// order of their linear index.
	const std::vector<std::size_t> extents = { 17, 13 };
	const std::vector<operand_spec> operands = { { { 1, 17 }, 0 } };
	const std::size_t count = 17 * 13;

	thread_pool pool(4);
	const loop_schedule schedule(pool, 1);

	std::vector<std::ptrdiff_t> linear(count, -1);
	std::vector<std::ptrdiff_t> coordinates(count, -1);
	run_indexed_elementwise_loop(
		linear_index_writer(),
		make_indexed_layout(extents, operands, linear_index_tag()),
		linear_index_tag(),
		schedule,
		linear.data()
	);
	run_indexed_elementwise_loop(
		coordinate_writer(),
		make_indexed_layout(extents, operands, multidimensional_index_tag()),
		multidimensional_index_tag(),
		schedule,
		coordinates.data()
	);

	for (std::size_t i = 0; i < 17; ++i)
	{
		for (std::size_t j = 0; j < 13; ++j)
		{
			const auto element = i + 17*j;
			CHECK( linear[element] == static_cast<std::ptrdiff_t>(13*i + j) );
			CHECK(
				coordinates[element] ==
				static_cast<std::ptrdiff_t>(100*i + j)
			);
		}
	}
}

TEST_CASE(
	"run_indexed_elementwise_vector_loop should place the linear index at the "
	"first element of every vector",
	"[indexed_elementwise_vector_loop]"
)
{
	auto buffer = make_buffer();
	std::vector<linear_vector_call> calls;

	SECTION( "a row-major operand, walked as a single vector" )
	{
		run_indexed_elementwise_vector_loop(
			linear_vector_recorder(calls),
			make_indexed_layout(
				{ 2, 3 },
				{ { { 3, 1 }, 0 } },
				linear_index_tag()
			),
			linear_index_tag(),
			buffer.data()
		);

		const std::vector<linear_vector_call> expected = {
			{ 6, 0, 1, true }
		};
		CHECK( calls == expected );
	}
	SECTION( "a column-major operand, walked column by column" )
	{
		run_indexed_elementwise_vector_loop(
			linear_vector_recorder(calls),
			make_indexed_layout(
				{ 2, 3 },
				{ { { 1, 2 }, 0 } },
				linear_index_tag()
			),
			linear_index_tag(),
			buffer.data()
		);

		const std::vector<linear_vector_call> expected = {
			{ 2, 0, 3, false },
			{ 2, 1, 4, false },
			{ 2, 2, 5, false }
		};
		CHECK( calls == expected );
	}
}

TEST_CASE(
	"run_indexed_elementwise_vector_loop should place the coordinates at the "
	"first element of every vector",
	"[indexed_elementwise_vector_loop]"
)
{
	// The vectors of a column-major 2x3 array are its columns.
	auto buffer = make_buffer();
	std::vector<std::vector<std::size_t>> calls;

	run_indexed_elementwise_vector_loop(
		[&calls] (
			const auto& /*pointers*/,
			const auto& /*strides*/,
			std::size_t count,
			const multidimensional_index &index
		)
		{
			const auto last = index.advanced(count - 1);
			calls.push_back({ count, index[0], index[1], last[0], last[1] });
		},
		make_indexed_layout(
			{ 2, 3 },
			{ { { 1, 2 }, 0 } },
			multidimensional_index_tag()
		),
		multidimensional_index_tag(),
		buffer.data()
	);

	const std::vector<std::vector<std::size_t>> expected = {
		{ 2, 0, 0, 1, 0 },
		{ 2, 0, 1, 1, 1 },
		{ 2, 0, 2, 1, 2 }
	};
	CHECK( calls == expected );
}
