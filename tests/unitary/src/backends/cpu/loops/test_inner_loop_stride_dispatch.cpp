// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/loops/inner_loop_stride_dispatch.hpp>

#include <xmipp4/core/layout/joint_layout.hpp>
#include <xmipp4/core/layout/joint_layout_builder.hpp>
#include <xmipp4/core/span.hpp>

#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

using namespace rex;
using namespace rex::cpu;

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
 * @brief Description of a single stride tag received by the visitor.
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
 * @brief Visitor recording the stride tags it is invoked with.
 *
 * It flattens the tuple produced by dispatch_inner_loop_strides into a
 * vector of stride_info so that the resolved tags can be inspected. The
 * recorded vector is returned so that the return-value plumbing of
 * dispatch_inner_loop_strides is exercised as well.
 */
struct recording_visitor
{
	template <typename... Tags>
	std::vector<stride_info> operator()(const std::tuple<Tags...> &strides) const
	{
		std::vector<stride_info> result;
		record(result, strides, std::index_sequence_for<Tags...>());
		return result;
	}

	template <typename Tuple, std::size_t... Is>
	static void record(
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
 * @brief Build a layout with the provided iteration extents and one operand
 * per stride vector.
 *
 * Optimizations are disabled during the build so that the strides are kept
 * verbatim and their order (hence the inner-most stride at index 0) is
 * predictable.
 */
joint_layout make_layout(
	const std::vector<std::size_t> &extents,
	const std::vector<std::vector<std::ptrdiff_t>> &operand_strides
)
{
	joint_layout_builder builder;
	builder.set_extents(make_span(extents));
	for (const auto &strides : operand_strides)
	{
		builder.add_operand(make_span(extents), make_span(strides), 0);
	}
	return builder.build({});
}

} // namespace


TEST_CASE(
	"dispatch_inner_loop_strides resolves a unit inner stride to a contiguous "
	"tag",
	"[inner_loop_stride_dispatch]"
)
{
	const auto layout = make_layout({4}, {{1}});

	const auto result = dispatch_inner_loop_strides(
		recording_visitor(),
		layout,
		std::integral_constant<std::size_t, 1>()
	);

	REQUIRE( result.size() == 1 );
	CHECK( result[0] == stride_info{1, stride_kind::contiguous} );
}

TEST_CASE(
	"dispatch_inner_loop_strides resolves a zero inner stride to a "
	"broadcasting tag",
	"[inner_loop_stride_dispatch]"
)
{
	const auto layout = make_layout({4}, {{0}});

	const auto result = dispatch_inner_loop_strides(
		recording_visitor(),
		layout,
		std::integral_constant<std::size_t, 1>()
	);

	REQUIRE( result.size() == 1 );
	CHECK( result[0] == stride_info{0, stride_kind::broadcasting} );
}

TEST_CASE(
	"dispatch_inner_loop_strides forwards a generic inner stride as a "
	"runtime value",
	"[inner_loop_stride_dispatch]"
)
{
	const auto layout = make_layout({4}, {{7}});

	const auto result = dispatch_inner_loop_strides(
		recording_visitor(),
		layout,
		std::integral_constant<std::size_t, 1>()
	);

	REQUIRE( result.size() == 1 );
	CHECK( result[0] == stride_info{7, stride_kind::runtime} );
}

TEST_CASE(
	"dispatch_inner_loop_strides resolves each operand independently",
	"[inner_loop_stride_dispatch]"
)
{
	// Three operands with contiguous, broadcasting and generic inner strides.
	const auto layout = make_layout({4}, {{1}, {0}, {7}});

	const auto result = dispatch_inner_loop_strides(
		recording_visitor(),
		layout,
		std::integral_constant<std::size_t, 3>()
	);

	REQUIRE( result.size() == 3 );
	CHECK( result[0] == stride_info{1, stride_kind::contiguous} );
	CHECK( result[1] == stride_info{0, stride_kind::broadcasting} );
	CHECK( result[2] == stride_info{7, stride_kind::runtime} );
}

TEST_CASE(
	"dispatch_inner_loop_strides only inspects the inner-most stride of "
	"each operand",
	"[inner_loop_stride_dispatch]"
)
{
	// Only the stride at index 0 must be used; the outer strides are ignored.
	const auto layout = make_layout(
		{4, 4},
		{{1, 100}, {0, 200}}
	);

	const auto result = dispatch_inner_loop_strides(
		recording_visitor(),
		layout,
		std::integral_constant<std::size_t, 2>()
	);

	REQUIRE( result.size() == 2 );
	CHECK( result[0] == stride_info{1, stride_kind::contiguous} );
	CHECK( result[1] == stride_info{0, stride_kind::broadcasting} );
}

TEST_CASE(
	"dispatch_inner_loop_strides with a rank-0 layout invokes the callable with "
	"broadcasting tags for every operand",
	"[inner_loop_stride_dispatch]"
)
{
	// A layout with empty extents has no strides to inspect, so every operand
	// collapses to a broadcasting (zero-stride) tag.
	const auto layout = make_layout({}, {});

	const auto result = dispatch_inner_loop_strides(
		recording_visitor(),
		layout,
		std::integral_constant<std::size_t, 2>()
	);

	REQUIRE( result.size() == 2 );
	CHECK( result[0] == stride_info{0, stride_kind::broadcasting} );
	CHECK( result[1] == stride_info{0, stride_kind::broadcasting} );
}

TEST_CASE(
	"dispatch_inner_loop_strides with an index sequence selects the "
	"requested operands in the given order",
	"[inner_loop_stride_dispatch]"
)
{
	const auto layout = make_layout({4}, {{1}, {5}, {0}});

	// Pick operands 2 and 0, in that order.
	const auto result = dispatch_inner_loop_strides(
		recording_visitor(),
		layout,
		std::index_sequence<2, 0>()
	);

	REQUIRE( result.size() == 2 );
	CHECK( result[0] == stride_info{0, stride_kind::broadcasting} );
	CHECK( result[1] == stride_info{1, stride_kind::contiguous} );
}

TEST_CASE(
	"dispatch_inner_loop_strides with an empty index sequence produces an "
	"empty tuple",
	"[inner_loop_stride_dispatch]"
)
{
	const auto layout = make_layout({4}, {{1}});

	const auto result = dispatch_inner_loop_strides(
		recording_visitor(),
		layout,
		std::index_sequence<>()
	);

	CHECK( result.empty() );
}
