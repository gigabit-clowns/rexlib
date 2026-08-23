// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/loops/inner_loop_stride_dispatch.hpp>
#include <backends/cpu/loops/reduction_element_adaptor.hpp>

#include <xmipp4/core/meta/type_list.hpp>
#include <xmipp4/core/platform/constexpr.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <tuple>
#include <vector>

using namespace xmipp4;
using namespace xmipp4::cpu;

namespace
{

/**
 * @brief The order the elements of a run reached the kernel.
 */
struct fold_log
{
	std::vector<std::size_t> seeded;
	std::vector<std::size_t> combined;
	std::size_t merges = 0;
};

/**
 * @brief Summing element kernel that records the positions it is handed.
 *
 * Says nothing about reassociation, so the adaptor keeps its runs in order.
 */
class recording_fold_kernel
{
public:
	recording_fold_kernel()
		: m_log(std::make_shared<fold_log>())
	{
	}

	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<int>;
	};

	void seed(int &accumulator, const int *value, std::size_t position) const
	{
		m_log->seeded.push_back(position);
		accumulator = *value;
	}

	void combine(
		int &accumulator,
		const int *value,
		std::size_t position
	) const
	{
		m_log->combined.push_back(position);
		accumulator += *value;
	}

	void merge(int &accumulator, const int &other) const
	{
		++m_log->merges;
		accumulator += other;
	}

	void finalize(int *result, const int &accumulator, std::size_t) const
	{
		*result = accumulator;
	}

	const fold_log& log() const noexcept
	{
		return *m_log;
	}

private:
	std::shared_ptr<fold_log> m_log;
};

/**
 * @brief The same kernel, allowing its runs to be dealt out over lanes.
 *
 * Identical in every other respect, which is what makes the answer the two
 * arrive at comparable.
 */
class reassociable_fold_kernel : public recording_fold_kernel
{
public:
	static XMIPP4_CONST_CONSTEXPR bool reassociable_fold = true;
};

/**
 * @brief A kernel that says its fold may not be reassociated by saying so.
 */
struct explicitly_serial_kernel
{
	static XMIPP4_CONST_CONSTEXPR bool reassociable_fold = false;
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

XMIPP4_CONST_CONSTEXPR std::size_t int_lane_count =
	reduction_fold_lane_count<type_list<int>>::value;

XMIPP4_CONST_CONSTEXPR std::size_t strip_block =
	reduction_strip_block_size<type_list<int>>::value;

std::vector<int> iota_vector(std::size_t count)
{
	std::vector<int> result(count);
	std::iota(result.begin(), result.end(), 1);
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
	CHECK(
		!has_reassociable_fold<recording_fold_kernel>::value
	);
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
	// An operation reporting where it found something depends on this: the
	// earliest of two equally good answers is only the earliest one if the
	// elements arrived in the order they sit in.
	const auto values = iota_vector(4*int_lane_count);
	const recording_fold_kernel kernel;
	const auto adaptor = make_reduction_element_adaptor(kernel);

	int accumulator = 0;
	adaptor.combine_run(
		std::make_tuple(&accumulator),
		std::make_tuple(values.data()),
		std::make_tuple(contiguous_stride_tag()),
		values.size(),
		0
	);

	std::vector<std::size_t> expected(values.size());
	std::iota(expected.begin(), expected.end(), 0);

	REQUIRE( kernel.log().combined == expected );
	REQUIRE( kernel.log().seeded.empty() );
	REQUIRE( kernel.log().merges == 0 );
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
	const auto values = iota_vector(4*int_lane_count);
	const reassociable_fold_kernel kernel;
	const auto adaptor = make_reduction_element_adaptor(kernel);

	int accumulator = 0;
	adaptor.combine_run(
		std::make_tuple(&accumulator),
		std::make_tuple(values.data()),
		std::make_tuple(contiguous_stride_tag()),
		values.size(),
		0
	);

	SECTION( "every element is folded exactly once" )
	{
		// The answer is what a single accumulator arrives at, reached by a
		// different route: each element enters one lane, and every lane is
		// merged back.
		auto visited = kernel.log().seeded;
		visited.insert(
			visited.end(),
			kernel.log().combined.cbegin(),
			kernel.log().combined.cend()
		);
		std::sort(visited.begin(), visited.end());

		std::vector<std::size_t> expected(values.size());
		std::iota(expected.begin(), expected.end(), 0);
		REQUIRE( visited == expected );
	}

	SECTION( "the first elements seed one lane each" )
	{
		std::vector<std::size_t> expected(int_lane_count);
		std::iota(expected.begin(), expected.end(), 0);
		REQUIRE( kernel.log().seeded == expected );
	}

	SECTION( "consecutive elements go to consecutive lanes" )
	{
		// What makes one deal one contiguous stretch of the input, which is
		// the whole point of dealing them this way rather than in blocks.
		REQUIRE( kernel.log().combined.size() >= 2 );
		REQUIRE( kernel.log().combined[0] == int_lane_count );
		REQUIRE( kernel.log().combined[1] == int_lane_count + 1 );
	}

	SECTION( "the lanes are merged back into the accumulator" )
	{
		REQUIRE( kernel.log().merges == int_lane_count );
	}

	SECTION( "the answer is the one a serial fold arrives at" )
	{
		// Exact arithmetic here, so the reassociation is invisible and the
		// two may be compared without a tolerance.
		REQUIRE(
			accumulator == std::accumulate(values.cbegin(), values.cend(), 0)
		);
	}
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
	const reassociable_fold_kernel kernel;
	const auto adaptor = make_reduction_element_adaptor(kernel);

	int accumulator = 0;
	adaptor.combine_run(
		std::make_tuple(&accumulator),
		std::make_tuple(values.data()),
		std::make_tuple(contiguous_stride_tag()),
		values.size(),
		0
	);

	std::vector<std::size_t> expected(values.size());
	std::iota(expected.begin(), expected.end(), 0);

	REQUIRE( kernel.log().combined == expected );
	REQUIRE( kernel.log().seeded.empty() );
	REQUIRE( kernel.log().merges == 0 );
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
		const auto values = iota_vector(count);
		const reassociable_fold_kernel kernel;
		const auto adaptor = make_reduction_element_adaptor(kernel);

		int accumulator = 0;
		adaptor.combine_run(
			std::make_tuple(&accumulator),
			std::make_tuple(values.data()),
			std::make_tuple(contiguous_stride_tag()),
			count,
			0
		);

		const auto expected =
			std::accumulate(values.cbegin(), values.cend(), 0);
		REQUIRE( accumulator == expected );
		REQUIRE(
			kernel.log().seeded.size() + kernel.log().combined.size() == count
		);
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

	SECTION( "a kernel folding in order" )
	{
		const recording_fold_kernel kernel;
		int accumulator = 0;
		make_reduction_element_adaptor(kernel).combine_run(
			std::make_tuple(&accumulator),
			std::make_tuple(values.data()),
			std::make_tuple(std::ptrdiff_t(2)),
			values.size() / 2,
			0
		);

		int expected = 0;
		for (std::size_t i = 0; i < values.size(); i += 2)
		{
			expected += values[i];
		}
		REQUIRE( accumulator == expected );
	}

	SECTION( "a kernel folding in lanes" )
	{
		const reassociable_fold_kernel kernel;
		int accumulator = 0;
		make_reduction_element_adaptor(kernel).combine_run(
			std::make_tuple(&accumulator),
			std::make_tuple(values.data()),
			std::make_tuple(std::ptrdiff_t(2)),
			values.size() / 2,
			0
		);

		int expected = 0;
		for (std::size_t i = 0; i < values.size(); i += 2)
		{
			expected += values[i];
		}
		REQUIRE( accumulator == expected );
	}
}

TEST_CASE(
	"reduction_element_adaptor should report positions relative to where the "
	"run starts",
	"[reduction_element_adaptor]"
)
{
	// A run is a stretch of the reduced space rather than the whole of it,
	// so the position it reports has to be the absolute one. The merge that
	// follows a fold split depends on it, and so does anything reporting a
	// location.
	XMIPP4_CONST_CONSTEXPR std::size_t offset = 100;
	const auto values = iota_vector(3);
	const recording_fold_kernel kernel;

	int accumulator = 0;
	make_reduction_element_adaptor(kernel).combine_run(
		std::make_tuple(&accumulator),
		std::make_tuple(values.data()),
		std::make_tuple(contiguous_stride_tag()),
		values.size(),
		offset
	);

	REQUIRE(
		kernel.log().combined ==
		std::vector<std::size_t>{ offset, offset + 1, offset + 2 }
	);
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
	XMIPP4_CONST_CONSTEXPR std::size_t width = 4;
	XMIPP4_CONST_CONSTEXPR std::size_t count = 3;

	// A count by width matrix, row major, so a step along the run is `width`
	// elements and a step along the strip is one.
	const auto values = iota_vector(count*width);
	const reassociable_fold_kernel kernel;

	std::vector<int> accumulators(width, 10);
	make_reduction_element_adaptor(kernel).combine_strip(
		std::make_tuple(accumulators.data()),
		std::make_tuple(values.data()),
		std::make_tuple(contiguous_stride_tag()),
		std::make_tuple(std::ptrdiff_t(width)),
		width,
		count,
		7
	);

	// Column j holds 10 plus the column's three elements.
	REQUIRE(
		accumulators ==
		std::vector<int>{ 10+1+5+9, 10+2+6+10, 10+3+7+11, 10+4+8+12 }
	);
	REQUIRE( kernel.log().merges == 0 );

	// Every element of the strip at one place in the run shares its position.
	auto positions = kernel.log().combined;
	std::sort(positions.begin(), positions.end());
	std::vector<std::size_t> expected;
	for (std::size_t e = 0; e < count; ++e)
	{
		expected.insert(expected.end(), width, 7 + e);
	}
	REQUIRE( positions == expected );
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
	XMIPP4_CONST_CONSTEXPR std::size_t count = 3;
	XMIPP4_CONST_CONSTEXPR std::size_t widest = 4*strip_block + 3;
	const auto values = iota_vector(count*widest);

	for (std::size_t width = 1; width <= widest; ++width)
	{
		const recording_fold_kernel kernel;
		std::vector<int> accumulators(widest + 1, 0);
		accumulators.back() = -1; // A guard, to catch a block running over.

		make_reduction_element_adaptor(kernel).combine_strip(
			std::make_tuple(accumulators.data()),
			std::make_tuple(values.data()),
			std::make_tuple(contiguous_stride_tag()),
			std::make_tuple(std::ptrdiff_t(widest)),
			width,
			count,
			0
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
	XMIPP4_CONST_CONSTEXPR std::size_t width = 3;
	const auto values = iota_vector(width);
	const recording_fold_kernel kernel;

	std::vector<int> accumulators(width, 0);
	make_reduction_element_adaptor(kernel).combine_strip(
		std::make_tuple(accumulators.data()),
		std::make_tuple(values.data()),
		std::make_tuple(broadcasting_stride_tag()),
		std::make_tuple(broadcasting_stride_tag()),
		width,
		1,
		0
	);

	REQUIRE( accumulators == std::vector<int>{ 1, 1, 1 } );
}
