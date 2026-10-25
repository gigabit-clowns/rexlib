// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/loops/inner_loop_stride_dispatch.hpp>
#include <backends/cpu/loops/linear_index_run.hpp>
#include <backends/cpu/loops/reduction_element_adaptor.hpp>

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <trompeloeil.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <tuple>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;
using trompeloeil::_;

namespace
{

class mock_fold_kernel
{
public:
	MAKE_CONST_MOCK1(seed, void(std::size_t index));
	MAKE_CONST_MOCK1(combine, void(std::size_t index));
	MAKE_CONST_MOCK0(merge, void());
};

/**
 * @brief Summing element kernel that reports the indices it is handed, and
 * its merges, to a mock.
 *
 * Says nothing about reassociation, so the adaptor keeps its runs in order.
 */
class forwarding_fold_kernel
{
public:
	explicit forwarding_fold_kernel(const mock_fold_kernel &mock) noexcept
		: m_mock(&mock)
	{
	}

	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<int>;
	};

	void seed(int &accumulator, const int *value, std::size_t index) const
	{
		m_mock->seed(index);
		accumulator = *value;
	}

	void combine(
		int &accumulator,
		const int *value,
		std::size_t index
	) const
	{
		m_mock->combine(index);
		accumulator += *value;
	}

	void merge(int &accumulator, const int &other) const
	{
		m_mock->merge();
		accumulator += other;
	}

	void finalize(int *result, const int &accumulator, std::size_t) const
	{
		*result = accumulator;
	}

private:
	const mock_fold_kernel *m_mock;
};

/**
 * @brief The same kernel, allowing its runs to be dealt out over lanes.
 *
 * Identical in every other respect, which is what makes the answer the two
 * arrive at comparable.
 */
class reassociable_fold_kernel : public forwarding_fold_kernel
{
public:
	using forwarding_fold_kernel::forwarding_fold_kernel;

	static REXLIB_CONST_CONSTEXPR bool reassociable_fold = true;
};

using expectations = std::vector<std::unique_ptr<trompeloeil::expectation>>;

/**
 * @brief Expect a run folded in order: one combine per index.
 */
void expect_serial_fold(
	expectations &expected,
	const mock_fold_kernel &kernel,
	trompeloeil::sequence &order,
	const std::vector<std::size_t> &indices
)
{
	for (const auto index : indices)
	{
		expected.push_back(
			NAMED_REQUIRE_CALL(kernel, combine(index)).IN_SEQUENCE(order)
		);
	}
}

/**
 * @brief Expect a run of @p count elements from index 0 dealt over lanes.
 *
 * One seed per lane, the remaining elements combined in order, and one merge
 * per lane.
 */
void expect_dealt_fold(
	expectations &expected,
	const mock_fold_kernel &kernel,
	trompeloeil::sequence &order,
	std::size_t lanes,
	std::size_t count
)
{
	for (std::size_t index = 0; index < lanes; ++index)
	{
		expected.push_back(
			NAMED_REQUIRE_CALL(kernel, seed(index)).IN_SEQUENCE(order)
		);
	}
	for (std::size_t index = lanes; index < count; ++index)
	{
		expected.push_back(
			NAMED_REQUIRE_CALL(kernel, combine(index)).IN_SEQUENCE(order)
		);
	}
	for (std::size_t lane = 0; lane < lanes; ++lane)
	{
		expected.push_back(
			NAMED_REQUIRE_CALL(kernel, merge()).IN_SEQUENCE(order)
		);
	}
}

/**
 * @brief Summing element kernel that takes no index at all.
 */
struct unindexed_fold_kernel
{
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<int>;
	};

	void seed(int &accumulator, const int *value) const
	{
		accumulator = *value;
	}

	void combine(int &accumulator, const int *value) const
	{
		accumulator += *value;
	}

	void merge(int &accumulator, const int &other) const
	{
		accumulator += other;
	}
};

/**
 * @brief A kernel that says its fold may not be reassociated by saying so.
 */
struct explicitly_serial_kernel
{
	static REXLIB_CONST_CONSTEXPR bool reassociable_fold = false;
};

/**
 * @brief A kernel with a neutral element, to be seen through the adaptor.
 */
struct kernel_with_identity
{
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<int>;
	};

	void identity(int &accumulator) const noexcept
	{
		accumulator = 0;
	}
};

/**
 * @brief The same without one.
 */
struct kernel_without_identity
{
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<int>;
	};
};

REXLIB_CONST_CONSTEXPR std::size_t int_lane_count =
	reduction_fold_lane_count<type_list<int>>::value;

REXLIB_CONST_CONSTEXPR std::size_t strip_block =
	reduction_strip_block_size<type_list<int>>::value;

std::vector<int> iota_vector(std::size_t count)
{
	std::vector<int> result(count);
	std::iota(result.begin(), result.end(), 1);
	return result;
}

/**
 * @brief An index run at a linear index, advancing by one per element.
 */
linear_index_run<contiguous_stride_tag> index_run_at(std::size_t index)
{
	return linear_index_run<contiguous_stride_tag>(
		static_cast<std::ptrdiff_t>(index),
		contiguous_stride_tag()
	);
}

std::vector<std::size_t> iota_indices(std::size_t count)
{
	std::vector<std::size_t> result(count);
	std::iota(result.begin(), result.end(), std::size_t(0));
	return result;
}

} // anonymous namespace

TEST_CASE(
	"has_reassociable_fold should only hold for a kernel that says so",
	"[reduction_element_adaptor]"
)
{
	// Saying nothing is the answer that costs nothing, so it has to be the
	// one that keeps the fold in order.
	CHECK( has_reassociable_fold<reassociable_fold_kernel>::value );
	CHECK( !has_reassociable_fold<forwarding_fold_kernel>::value );
	CHECK( !has_reassociable_fold<explicitly_serial_kernel>::value );
	CHECK( !has_reassociable_fold<kernel_with_identity>::value );
}

TEST_CASE(
	"reduction_fold_lane_count should shrink as the accumulators grow",
	"[reduction_element_adaptor]"
)
{
	// Lanes are held in registers or close to them, so a kernel keeping more
	// of them, or larger ones, is dealt fewer rather than spilling.
	using one_small = type_list<std::int8_t>;
	using one_large = type_list<double>;
	using many_large = type_list<double, double, double, double>;

	CHECK(
		reduction_fold_lane_count<one_small>::value >=
		reduction_fold_lane_count<one_large>::value
	);
	CHECK(
		reduction_fold_lane_count<one_large>::value >=
		reduction_fold_lane_count<many_large>::value
	);

	// However large they get, there is always more than one lane, and never
	// more than the measurement supports.
	CHECK( reduction_fold_lane_count<many_large>::value >= 2 );
	CHECK( reduction_fold_lane_count<one_small>::value <= 16 );
}

TEST_CASE(
	"reduction_element_adaptor should answer about the identity as the "
	"kernel it adapts does",
	"[reduction_element_adaptor]"
)
{
	// The loop asks the adaptor, not what is underneath it, so an identity
	// that stopped being visible through it would turn an answerable empty
	// reduction into a rejected one.
	using with = reduction_element_adaptor<kernel_with_identity>;
	using without = reduction_element_adaptor<kernel_without_identity>;

	CHECK( has_reduction_identity<with, type_list<int>>::value );
	CHECK( !has_reduction_identity<without, type_list<int>>::value );
}

TEST_CASE(
	"reduction_element_adaptor should fold a run in order when the kernel "
	"does not allow reassociation",
	"[reduction_element_adaptor]"
)
{
	// The elements reach the kernel in the order they sit in, which is what
	// a fold that is not allowed to reassociate commits to.
	const auto values = iota_vector(4*int_lane_count);
	const mock_fold_kernel kernel;
	const auto adaptor =
		make_reduction_element_adaptor(forwarding_fold_kernel(kernel));

	trompeloeil::sequence order;
	expectations expected;
	expect_serial_fold(expected, kernel, order, iota_indices(values.size()));
	FORBID_CALL(kernel, seed(_));
	FORBID_CALL(kernel, merge());

	int accumulator = 0;
	adaptor.combine_run(
		std::make_tuple(&accumulator),
		std::make_tuple(values.data()),
		std::make_tuple(contiguous_stride_tag()),
		values.size(),
		index_run_at(0)
	);

	REQUIRE(
		accumulator == std::accumulate(values.cbegin(), values.cend(), 0)
	);
}

TEST_CASE(
	"reduction_element_adaptor should deal a run out over lanes when the "
	"kernel allows reassociation",
	"[reduction_element_adaptor]"
)
{
	// Each lane is seeded from an element of its own, consecutive elements
	// then go to consecutive lanes, and every lane is merged back.
	const auto values = iota_vector(4*int_lane_count);
	const mock_fold_kernel kernel;
	const auto adaptor =
		make_reduction_element_adaptor(reassociable_fold_kernel(kernel));

	trompeloeil::sequence order;
	expectations expected;
	expect_dealt_fold(
		expected,
		kernel,
		order,
		int_lane_count,
		values.size()
	);

	int accumulator = 0;
	adaptor.combine_run(
		std::make_tuple(&accumulator),
		std::make_tuple(values.data()),
		std::make_tuple(contiguous_stride_tag()),
		values.size(),
		index_run_at(0)
	);

	// Exact arithmetic here, so the reassociation is invisible and the
	// answer may be compared with a serial fold's without a tolerance.
	REQUIRE(
		accumulator == std::accumulate(values.cbegin(), values.cend(), 0)
	);
}

TEST_CASE(
	"reduction_element_adaptor should fold a run too short to deal serially",
	"[reduction_element_adaptor]"
)
{
	// Seeding a lane from every element and merging them all back would cost
	// more than the independence buys, so a short run keeps the simple path
	// even for a kernel that would allow the other one.
	const auto values = iota_vector(2*int_lane_count - 1);
	const mock_fold_kernel kernel;
	const auto adaptor =
		make_reduction_element_adaptor(reassociable_fold_kernel(kernel));

	trompeloeil::sequence order;
	expectations expected;
	expect_serial_fold(expected, kernel, order, iota_indices(values.size()));
	FORBID_CALL(kernel, seed(_));
	FORBID_CALL(kernel, merge());

	int accumulator = 0;
	adaptor.combine_run(
		std::make_tuple(&accumulator),
		std::make_tuple(values.data()),
		std::make_tuple(contiguous_stride_tag()),
		values.size(),
		index_run_at(0)
	);
}

TEST_CASE(
	"reduction_element_adaptor should fold every element of a run whatever "
	"its length",
	"[reduction_element_adaptor]"
)
{
	// A run is only a whole number of deals by accident, so the remainder
	// has to reach an accumulator too. Sweeping across the length at which
	// the lanes are taken up catches a tail that is dropped or folded twice.
	for (std::size_t count = 1; count <= 5*int_lane_count; ++count)
	{
		INFO( "count " << count );
		const auto values = iota_vector(count);
		const mock_fold_kernel kernel;
		const auto adaptor =
			make_reduction_element_adaptor(reassociable_fold_kernel(kernel));

		trompeloeil::sequence order;
		expectations expected;
		if (count < 2*int_lane_count)
		{
			expect_serial_fold(expected, kernel, order, iota_indices(count));
		}
		else
		{
			expect_dealt_fold(expected, kernel, order, int_lane_count, count);
		}

		int accumulator = 0;
		adaptor.combine_run(
			std::make_tuple(&accumulator),
			std::make_tuple(values.data()),
			std::make_tuple(contiguous_stride_tag()),
			count,
			index_run_at(0)
		);

		const auto expected_total =
			std::accumulate(values.cbegin(), values.cend(), 0);
		REQUIRE( accumulator == expected_total );
	}
}

TEST_CASE(
	"reduction_element_adaptor should follow the stride of a run",
	"[reduction_element_adaptor]"
)
{
	// The run is a stretch of the reduced space, not of memory, so a strided
	// or reversed input has to be walked by its own stride on both paths.
	const auto values = iota_vector(4*int_lane_count);

	int total = 0;
	for (std::size_t i = 0; i < values.size(); i += 2)
	{
		total += values[i];
	}

	const mock_fold_kernel kernel;
	trompeloeil::sequence order;
	expectations expected;

	SECTION( "a kernel folding in order" )
	{
		expect_serial_fold(
			expected,
			kernel,
			order,
			iota_indices(values.size() / 2)
		);

		int accumulator = 0;
		make_reduction_element_adaptor(
			forwarding_fold_kernel(kernel)
		).combine_run(
			std::make_tuple(&accumulator),
			std::make_tuple(values.data()),
			std::make_tuple(std::ptrdiff_t(2)),
			values.size() / 2,
			index_run_at(0)
		);

		REQUIRE( accumulator == total );
	}

	SECTION( "a kernel folding in lanes" )
	{
		expect_dealt_fold(
			expected,
			kernel,
			order,
			int_lane_count,
			values.size() / 2
		);

		int accumulator = 0;
		make_reduction_element_adaptor(
			reassociable_fold_kernel(kernel)
		).combine_run(
			std::make_tuple(&accumulator),
			std::make_tuple(values.data()),
			std::make_tuple(std::ptrdiff_t(2)),
			values.size() / 2,
			index_run_at(0)
		);

		REQUIRE( accumulator == total );
	}
}

TEST_CASE(
	"reduction_element_adaptor should hand every element of a run the index "
	"its run places it at",
	"[reduction_element_adaptor]"
)
{
	// A run is a stretch of the reduced space rather than the whole of it,
	// and the index run it is handed says where that stretch sits and how
	// the index moves along it.
	const auto values = iota_vector(3);
	const mock_fold_kernel kernel;
	const auto adaptor =
		make_reduction_element_adaptor(forwarding_fold_kernel(kernel));
	trompeloeil::sequence order;
	expectations expected;

	SECTION( "a run advancing the index by one" )
	{
		expect_serial_fold(expected, kernel, order, { 100, 101, 102 });

		int accumulator = 0;
		adaptor.combine_run(
			std::make_tuple(&accumulator),
			std::make_tuple(values.data()),
			std::make_tuple(contiguous_stride_tag()),
			values.size(),
			index_run_at(100)
		);
	}

	SECTION( "a run advancing the index by more than one" )
	{
		// The shape of a run along an axis that is not the last one of the
		// reduced space.
		expect_serial_fold(expected, kernel, order, { 10, 13, 16 });

		int accumulator = 0;
		adaptor.combine_run(
			std::make_tuple(&accumulator),
			std::make_tuple(values.data()),
			std::make_tuple(contiguous_stride_tag()),
			values.size(),
			linear_index_run<std::ptrdiff_t>(10, 3)
		);
	}
}

TEST_CASE(
	"reduction_element_adaptor should hand no index to a kernel folded "
	"without an index run",
	"[reduction_element_adaptor]"
)
{
	const auto values = iota_vector(6);
	const auto adaptor = make_reduction_element_adaptor(unindexed_fold_kernel());

	SECTION( "along a run" )
	{
		int accumulator = 0;
		adaptor.combine_run(
			std::make_tuple(&accumulator),
			std::make_tuple(values.data()),
			std::make_tuple(contiguous_stride_tag()),
			values.size()
		);

		REQUIRE( accumulator == 21 );
	}

	SECTION( "across a strip" )
	{
		// A 2 by 3 matrix, row major, folded down its columns.
		std::vector<int> accumulators(3, 0);
		adaptor.combine_strip(
			std::make_tuple(accumulators.data()),
			std::make_tuple(values.data()),
			std::make_tuple(contiguous_stride_tag()),
			std::make_tuple(std::ptrdiff_t(3)),
			3,
			2
		);

		REQUIRE( accumulators == std::vector<int>{ 1+4, 2+5, 3+6 } );
	}
}

TEST_CASE(
	"reduction_element_adaptor should fold a run into every accumulator of a "
	"strip",
	"[reduction_element_adaptor]"
)
{
	// The other orientation: the accumulators are what is walked, and the
	// elements feeding them come from the kept layout while the run comes
	// from the reduced one. There are no lanes here, the strip being as many
	// independent accumulators as it is wide already.
	REXLIB_CONST_CONSTEXPR std::size_t width = 4;
	REXLIB_CONST_CONSTEXPR std::size_t count = 3;

	// A count by width matrix, row major, so a step along the run is `width`
	// elements and a step along the strip is one.
	const auto values = iota_vector(count*width);
	const mock_fold_kernel kernel;

	// Every element of the strip at one place in the run shares its index.
	expectations expected;
	for (std::size_t e = 0; e < count; ++e)
	{
		expected.push_back(
			NAMED_REQUIRE_CALL(kernel, combine(7 + e)).TIMES(width)
		);
	}
	FORBID_CALL(kernel, seed(_));
	FORBID_CALL(kernel, merge());

	std::vector<int> accumulators(width, 10);
	make_reduction_element_adaptor(
		reassociable_fold_kernel(kernel)
	).combine_strip(
		std::make_tuple(accumulators.data()),
		std::make_tuple(values.data()),
		std::make_tuple(contiguous_stride_tag()),
		std::make_tuple(std::ptrdiff_t(width)),
		width,
		count,
		index_run_at(7)
	);

	// Column j holds 10 plus the column's three elements.
	REQUIRE(
		accumulators ==
		std::vector<int>{ 10+1+5+9, 10+2+6+10, 10+3+7+11, 10+4+8+12 }
	);
}

TEST_CASE(
	"reduction_element_adaptor should fold a strip of any width",
	"[reduction_element_adaptor]"
)
{
	// A strip is walked in blocks of a size settled at compile time, so its
	// width is only a whole number of them by accident. Sweeping across the
	// width catches a block that is dropped, folded twice, or reads past the
	// accumulators it was given.
	REXLIB_CONST_CONSTEXPR std::size_t count = 3;
	REXLIB_CONST_CONSTEXPR std::size_t widest = 4*strip_block + 3;
	const auto values = iota_vector(count*widest);

	for (std::size_t width = 1; width <= widest; ++width)
	{
		const mock_fold_kernel kernel;
		ALLOW_CALL(kernel, combine(_)).WITH(_1 < count);

		std::vector<int> accumulators(widest + 1, 0);
		accumulators.back() = -1; // A guard, to catch a block running over.

		make_reduction_element_adaptor(
			forwarding_fold_kernel(kernel)
		).combine_strip(
			std::make_tuple(accumulators.data()),
			std::make_tuple(values.data()),
			std::make_tuple(contiguous_stride_tag()),
			std::make_tuple(std::ptrdiff_t(widest)),
			width,
			count,
			index_run_at(0)
		);

		INFO( "width " << width );
		for (std::size_t j = 0; j < width; ++j)
		{
			int expected = 0;
			for (std::size_t e = 0; e < count; ++e)
			{
				expected += values[e*widest + j];
			}
			REQUIRE( accumulators[j] == expected );
		}
		for (std::size_t j = width; j < widest; ++j)
		{
			REQUIRE( accumulators[j] == 0 );
		}
		REQUIRE( accumulators.back() == -1 );
	}
}

TEST_CASE(
	"reduction_element_adaptor should broadcast an operand across a strip",
	"[reduction_element_adaptor]"
)
{
	// A stride tag of zero leaves the operand where it is, so every
	// accumulator of the strip is fed the same element.
	REXLIB_CONST_CONSTEXPR std::size_t width = 3;
	const auto values = iota_vector(width);
	const mock_fold_kernel kernel;
	REQUIRE_CALL(kernel, combine(0u)).TIMES(width);

	std::vector<int> accumulators(width, 0);
	make_reduction_element_adaptor(
		forwarding_fold_kernel(kernel)
	).combine_strip(
		std::make_tuple(accumulators.data()),
		std::make_tuple(values.data()),
		std::make_tuple(broadcasting_stride_tag()),
		std::make_tuple(broadcasting_stride_tag()),
		width,
		1,
		index_run_at(0)
	);

	REQUIRE( accumulators == std::vector<int>{ 1, 1, 1 } );
}
