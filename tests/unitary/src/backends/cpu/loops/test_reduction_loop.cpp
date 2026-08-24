// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/config.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/loops/reduction_loop.hpp>

#include <rex/backends/cpu/thread_pool.hpp>
#include <rex/core/platform/constexpr.hpp>

#include <rex/core/layout/joint_layout.hpp>
#include <rex/core/layout/joint_layout_builder.hpp>
#include <rex/core/meta/type_list.hpp>
#include <rex/core/span.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

/**
 * @brief Build a layout from extents and one stride vector per operand.
 */
joint_layout make_layout(
	const std::vector<std::size_t> &extents,
	const std::vector<std::vector<std::ptrdiff_t>> &strides
)
{
	joint_layout_builder builder;
	builder.set_extents(make_span(extents));
	for (const auto &operand : strides)
	{
		builder.add_operand(
			make_span(extents),
			make_span(operand),
			0
		);
	}
	return builder.build();
}

/**
 * @brief Tally of the calls a kernel received.
 */
struct call_log
{
	std::size_t seeds = 0;
	std::size_t combines = 0;
	std::size_t finalizes = 0;
	std::size_t identities = 0;
	std::vector<std::size_t> reported_counts;
	std::vector<std::size_t> positions;
};

/**
 * @brief Summing kernel that records every call it receives.
 *
 * The log is held through a shared_ptr so that the copy the loop takes
 * shares state with the handle kept by the test.
 */
class recording_kernel
{
public:
	recording_kernel()
		: m_log(std::make_shared<call_log>())
	{
	}

	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<int>;
	};

	void seed(int &accumulator, const int *value, std::size_t position) const
	{
		++m_log->seeds;
		m_log->positions.push_back(position);
		accumulator = *value;
	}

	void combine(
		int &accumulator,
		const int *value,
		std::size_t position
	) const
	{
		++m_log->combines;
		m_log->positions.push_back(position);
		accumulator += *value;
	}

	void merge(int &accumulator, const int &other) const
	{
		accumulator += other;
	}

	void finalize(int *result, const int &accumulator, std::size_t count) const
	{
		++m_log->finalizes;
		m_log->reported_counts.push_back(count);
		*result = accumulator;
	}

	void identity(int &accumulator) const
	{
		++m_log->identities;
		accumulator = 0;
	}

	const call_log& log() const noexcept
	{
		return *m_log;
	}

private:
	std::shared_ptr<call_log> m_log;
};

/**
 * @brief Summing kernel with no neutral element.
 */
struct seeded_only_kernel
{
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<int>;
	};

	void seed(int &accumulator, const int *value, std::size_t) const
	{
		accumulator = *value;
	}

	void combine(int &accumulator, const int *value, std::size_t) const
	{
		accumulator += *value;
	}

	void merge(int &accumulator, const int &other) const
	{
		accumulator += other;
	}

	void finalize(int *result, const int &accumulator, std::size_t) const
	{
		*result = accumulator;
	}
};

/**
 * @brief Two accumulators of different types feeding one output.
 *
 * The shape a masked average has: a numerator and a denominator accumulated
 * apart and divided once the fold is over. No operation in the catalog needs
 * it yet, which is why it is exercised here.
 */
struct masked_mean_kernel
{
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<double, std::size_t>;
	};

	void seed(
		double &total,
		std::size_t &count,
		const double *value,
		const bool *mask,
		std::size_t
	) const
	{
		total = *mask ? *value : 0.0;
		count = *mask ? 1u : 0u;
	}

	void combine(
		double &total,
		std::size_t &count,
		const double *value,
		const bool *mask,
		std::size_t
	) const
	{
		if (*mask)
		{
			total += *value;
			++count;
		}
	}

	void merge(
		double &total,
		std::size_t &count,
		const double &other_total,
		const std::size_t &other_count
	) const
	{
		total += other_total;
		count += other_count;
	}

	void finalize(
		double *result,
		const double &total,
		const std::size_t &count,
		std::size_t
	) const
	{
		*result = count ? total / static_cast<double>(count) : 0.0;
	}
};

/**
 * @brief Two accumulators feeding two outputs in one pass.
 */
struct minmax_kernel
{
	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<int, int>;
	};

	void seed(
		int &lowest,
		int &highest,
		const int *value,
		std::size_t
	) const
	{
		lowest = *value;
		highest = *value;
	}

	void combine(
		int &lowest,
		int &highest,
		const int *value,
		std::size_t
	) const
	{
		lowest = std::min(lowest, *value);
		highest = std::max(highest, *value);
	}

	void merge(
		int &lowest,
		int &highest,
		const int &other_lowest,
		const int &other_highest
	) const
	{
		lowest = std::min(lowest, other_lowest);
		highest = std::max(highest, other_highest);
	}

	void finalize(
		int *lowest_result,
		int *highest_result,
		const int &lowest,
		const int &highest,
		std::size_t
	) const
	{
		*lowest_result = lowest;
		*highest_result = highest;
	}
};

std::vector<int> iota_vector(std::size_t count)
{
	std::vector<int> result(count);
	std::iota(result.begin(), result.end(), 0);
	return result;
}

} // anonymous namespace

TEST_CASE(
	"run_reduction_loop should seed once and combine once per remaining "
	"element for each output",
	"[reduction_loop]"
)
{
	// A 4x3 matrix reduced along its outer axis, so three outputs of four
	// elements each.
	const auto input = iota_vector(12);
	std::vector<int> output(3, -1);

	const recording_kernel kernel;
	run_reduction_loop(
		kernel,
		make_layout({3}, {{1}, {1}}),
		make_layout({4}, {{3}}),
		4,
		std::make_tuple(output.data()),
		std::make_tuple(static_cast<const int*>(input.data()))
	);

	CHECK( kernel.log().seeds == 3 );
	CHECK( kernel.log().combines == 3 * 3 );
	CHECK( kernel.log().finalizes == 3 );
	CHECK( kernel.log().identities == 0 );
	CHECK( kernel.log().reported_counts ==
	       std::vector<std::size_t>(3, std::size_t(4)) );

	CHECK( output == std::vector<int>({18, 22, 26}) );
}

TEST_CASE(
	"run_reduction_loop should fold along the innermost axis",
	"[reduction_loop]"
)
{
	// The same matrix reduced along its contiguous axis instead, which is
	// the orientation that leaves one accumulator held across a run.
	const auto input = iota_vector(12);
	std::vector<int> output(4, -1);

	const recording_kernel kernel;
	run_reduction_loop(
		kernel,
		make_layout({4}, {{3}, {1}}),
		make_layout({3}, {{1}}),
		3,
		std::make_tuple(output.data()),
		std::make_tuple(static_cast<const int*>(input.data()))
	);

	CHECK( kernel.log().seeds == 4 );
	CHECK( kernel.log().combines == 4 * 2 );
	CHECK( kernel.log().finalizes == 4 );
	CHECK( output == std::vector<int>({3, 12, 21, 30}) );
}

TEST_CASE(
	"run_reduction_loop should fold every axis into a single output",
	"[reduction_loop]"
)
{
	const auto input = iota_vector(12);
	int output = -1;

	const recording_kernel kernel;
	run_reduction_loop(
		kernel,
		make_layout({}, {{}, {}}),
		make_layout({4, 3}, {{3, 1}}),
		12,
		std::make_tuple(&output),
		std::make_tuple(static_cast<const int*>(input.data()))
	);

	CHECK( kernel.log().seeds == 1 );
	CHECK( kernel.log().combines == 11 );
	CHECK( kernel.log().finalizes == 1 );
	CHECK( output == 66 );
}

TEST_CASE(
	"run_reduction_loop should copy when no axis is reduced",
	"[reduction_loop]"
)
{
	// A reduced space of rank zero holds one position, so every output is
	// seeded and finalized without a single combination.
	const auto input = iota_vector(6);
	std::vector<int> output(6, -1);

	const recording_kernel kernel;
	run_reduction_loop(
		kernel,
		make_layout({6}, {{1}, {1}}),
		make_layout({}, {{}}),
		1,
		std::make_tuple(output.data()),
		std::make_tuple(static_cast<const int*>(input.data()))
	);

	CHECK( kernel.log().seeds == 6 );
	CHECK( kernel.log().combines == 0 );
	CHECK( kernel.log().finalizes == 6 );
	CHECK( output == input );
}

TEST_CASE(
	"run_reduction_loop should complete outputs beyond one tile",
	"[reduction_loop]"
)
{
	// More surviving elements than a tile holds, and a count that is not a
	// multiple of it, so the last tile is partial. Taken from the tile size
	// rather than written down, so that tuning the tile cannot quietly stop
	// this case from spanning more than one.
	const std::size_t rows = 3;
	const std::size_t tile = reduction_tile_size<type_list<int>>::value;
	const std::size_t columns = tile + tile / 2 + 7;

	const auto input = iota_vector(rows * columns);
	std::vector<int> output(columns, -1);

	const recording_kernel kernel;
	run_reduction_loop(
		kernel,
		make_layout({columns}, {{1}, {1}}),
		make_layout({rows}, {{static_cast<std::ptrdiff_t>(columns)}}),
		rows,
		std::make_tuple(output.data()),
		std::make_tuple(static_cast<const int*>(input.data()))
	);

	CHECK( kernel.log().seeds == columns );
	CHECK( kernel.log().combines == columns * (rows - 1) );
	CHECK( kernel.log().finalizes == columns );

	std::vector<int> expected(columns);
	for (std::size_t c = 0; c < columns; ++c)
	{
		int total = 0;
		for (std::size_t r = 0; r < rows; ++r)
		{
			total += input[r * columns + c];
		}
		expected[c] = total;
	}
	CHECK( output == expected );
}

TEST_CASE(
	"run_reduction_loop should follow the strides of each operand",
	"[reduction_loop]"
)
{
	const auto input = iota_vector(6);

	SECTION( "a reversed input" )
	{
		int output = -1;
		const recording_kernel kernel;
		run_reduction_loop(
			kernel,
			make_layout({}, {{}, {}}),
			make_layout({6}, {{-1}}),
			6,
			std::make_tuple(&output),
			std::make_tuple(static_cast<const int*>(input.data() + 5))
		);

		CHECK( output == 15 );
	}

	SECTION( "an input broadcast along the reduced axis" )
	{
		int output = -1;
		const recording_kernel kernel;
		run_reduction_loop(
			kernel,
			make_layout({}, {{}, {}}),
			make_layout({4}, {{0}}),
			4,
			std::make_tuple(&output),
			std::make_tuple(static_cast<const int*>(input.data() + 5))
		);

		CHECK( kernel.log().combines == 3 );
		CHECK( output == 20 );
	}

	SECTION( "an output written with a stride" )
	{
		std::vector<int> output(6, -1);
		const recording_kernel kernel;
		run_reduction_loop(
			kernel,
			make_layout({3}, {{1}, {2}}),
			make_layout({2}, {{3}}),
			2,
			std::make_tuple(output.data()),
			std::make_tuple(static_cast<const int*>(input.data()))
		);

		CHECK( output == std::vector<int>({3, -1, 5, -1, 7, -1}) );
	}
}

TEST_CASE(
	"run_reduction_loop should answer a reduction over no elements with the "
	"identity",
	"[reduction_loop]"
)
{
	std::vector<int> output(3, -1);

	const recording_kernel kernel;
	run_reduction_loop(
		kernel,
		make_layout({3}, {{1}, {1}}),
		make_layout({0}, {{1}}),
		0,
		std::make_tuple(output.data()),
		std::make_tuple(static_cast<const int*>(nullptr))
	);

	CHECK( kernel.log().seeds == 0 );
	CHECK( kernel.log().combines == 0 );
	CHECK( kernel.log().identities == 3 );
	CHECK( kernel.log().finalizes == 3 );
	CHECK( output == std::vector<int>({0, 0, 0}) );
}

TEST_CASE(
	"run_reduction_loop should reject a reduction over no elements without "
	"an identity",
	"[reduction_loop]"
)
{
	STATIC_REQUIRE(
		has_reduction_identity<recording_kernel, type_list<int>>::value
	);
	STATIC_REQUIRE_FALSE(
		has_reduction_identity<seeded_only_kernel, type_list<int>>::value
	);

	std::vector<int> output(3, -1);

	CHECK_THROWS_AS(
		run_reduction_loop(
			seeded_only_kernel(),
			make_layout({3}, {{1}, {1}}),
			make_layout({0}, {{1}}),
			0,
			std::make_tuple(output.data()),
			std::make_tuple(static_cast<const int*>(nullptr))
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"run_reduction_loop should do nothing when the outputs are empty",
	"[reduction_loop]"
)
{
	const auto input = iota_vector(6);

	const recording_kernel kernel;
	run_reduction_loop(
		kernel,
		make_layout({0}, {{1}, {1}}),
		make_layout({6}, {{1}}),
		6,
		std::make_tuple(static_cast<int*>(nullptr)),
		std::make_tuple(static_cast<const int*>(input.data()))
	);

	CHECK( kernel.log().seeds == 0 );
	CHECK( kernel.log().finalizes == 0 );
}

TEST_CASE(
	"run_reduction_loop should keep several accumulators apart",
	"[reduction_loop]"
)
{
	// Two accumulators of unrelated types, fed by two inputs and answering
	// through one output.
	const std::vector<double> values = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};

	// A plain array rather than a vector, std::vector<bool> having no
	// elements to point at.
	const bool mask[] = {true, false, true, true, false, true};

	std::vector<double> output(3, -1.0);

	run_reduction_loop(
		masked_mean_kernel(),
		make_layout({3}, {{1}, {1}, {1}}),
		make_layout({2}, {{3}, {3}}),
		2,
		std::make_tuple(output.data()),
		std::make_tuple(
			static_cast<const double*>(values.data()),
			static_cast<const bool*>(mask)
		)
	);

	// Column 0 keeps 1 and 4, column 1 keeps neither 2 nor 5, column 2 keeps
	// both 3 and 6.
	CHECK( output[0] == 2.5 );
	CHECK( output[1] == 0.0 );
	CHECK( output[2] == 4.5 );
}

TEST_CASE(
	"run_reduction_loop should write several outputs in one pass",
	"[reduction_loop]"
)
{
	const std::vector<int> input = {5, 1, 9, 3, 7, 2};
	std::vector<int> lowest(3, -1);
	std::vector<int> highest(3, -1);

	run_reduction_loop(
		minmax_kernel(),
		make_layout({3}, {{1}, {1}, {1}}),
		make_layout({2}, {{3}}),
		2,
		std::make_tuple(lowest.data(), highest.data()),
		std::make_tuple(static_cast<const int*>(input.data()))
	);

	CHECK( lowest == std::vector<int>({3, 1, 2}) );
	CHECK( highest == std::vector<int>({5, 7, 9}) );
}

TEST_CASE(
	"reduction_tile_size should shrink as the accumulators grow",
	"[reduction_loop]"
)
{
	// A tile is sized to keep its accumulators resident, so declaring more
	// of them, or larger ones, shortens it rather than spilling.
	using one_small = type_list<int>;
	using one_large = type_list<double>;
	using two_large = type_list<double, double>;

	CHECK( accumulator_footprint<one_small>::value == sizeof(int) );
	CHECK( accumulator_footprint<two_large>::value == 2 * sizeof(double) );

	CHECK( reduction_tile_size<one_large>::value <
	       reduction_tile_size<one_small>::value );
	CHECK( reduction_tile_size<two_large>::value <
	       reduction_tile_size<one_large>::value );
	CHECK( reduction_tile_size<two_large>::value > 0 );
}

namespace
{

/**
 * @brief Sum the inputs, and report where the largest of them sat.
 *
 * Two accumulators of different kinds on purpose: the sum is what floating
 * point reassociation would disturb, and the position is what a tie-break
 * would. Both have to survive the surviving space being shared out.
 */
struct sum_and_argmax_kernel
{
	template <typename Outputs, typename Inputs>
	struct accumulators;

	template <typename... Outs, typename In>
	struct accumulators<type_list<Outs...>, type_list<In>>
	{
		using type = type_list<double, std::int64_t, double>;
	};

	void seed(
		double &total,
		std::int64_t &where,
		double &best,
		const double *value,
		std::size_t position
	) const noexcept
	{
		total = *value;
		best = *value;
		where = static_cast<std::int64_t>(position);
	}

	void combine(
		double &total,
		std::int64_t &where,
		double &best,
		const double *value,
		std::size_t position
	) const noexcept
	{
		total += *value;
		if (*value > best)
		{
			best = *value;
			where = static_cast<std::int64_t>(position);
		}
	}

	void merge(
		double &total,
		std::int64_t &where,
		double &best,
		const double &other_total,
		const std::int64_t &other_where,
		const double &other_best
	) const noexcept
	{
		total += other_total;

		// Strict, so a tie keeps the incumbent. Merging in ascending slice
		// order then keeps the earliest position, which is what the serial
		// fold would have reported.
		if (other_best > best)
		{
			best = other_best;
			where = other_where;
		}
	}

	void finalize(
		double *total_out,
		std::int64_t *where_out,
		double &total,
		std::int64_t &where,
		double &/*best*/,
		std::size_t /*count*/
	) const noexcept
	{
		*total_out = total;
		*where_out = where;
	}
};

} // anonymous namespace

TEST_CASE(
	"run_reduction_loop should give the same answer however many threads "
	"fold the surviving space",
	"[reduction_loop]"
)
{
	// The surviving space is what gets shared out, so each output is still
	// folded start to finish by one thread and in the serial order. The
	// answer therefore has to match bit for bit, sums included, which is
	// what a tolerance here would hide rather than check.
	REXLIB_CONST_CONSTEXPR std::size_t kept = 37;
	REXLIB_CONST_CONSTEXPR std::size_t reduced = 11;

	std::vector<double> input(kept*reduced);
	for (std::size_t i = 0; i < input.size(); ++i)
	{
		// Values whose sum depends on the order they are added in, so that a
		// reassociated fold would be caught.
		input[i] = 1.0 / static_cast<double>(i + 1);
	}

	const auto kept_layout = make_layout(
		{ kept },
		{ { static_cast<std::ptrdiff_t>(reduced) }, { 1 }, { 1 } }
	);
	const auto reduced_layout = make_layout({ reduced }, { { 1 } });

	const auto fold =
		[&] (const loop_schedule &schedule,
		     std::vector<double> &totals,
		     std::vector<std::int64_t> &wheres)
		{
			totals.assign(kept, 0.0);
			wheres.assign(kept, -1);
			run_reduction_loop(
				sum_and_argmax_kernel(),
				kept_layout,
				reduced_layout,
				reduced,
				std::make_tuple(totals.data(), wheres.data()),
				std::make_tuple(
					static_cast<const double*>(input.data())
				),
				schedule
			);
		};

	std::vector<double> serial_totals;
	std::vector<std::int64_t> serial_wheres;
	fold(loop_schedule(), serial_totals, serial_wheres);

	// Every output was filled, so a silently skipped chunk would show.
	CHECK( std::count(serial_wheres.cbegin(), serial_wheres.cend(), -1) == 0 );

	for (std::size_t workers : { std::size_t(0), std::size_t(1),
	                             std::size_t(3), std::size_t(8) })
	{
		thread_pool pool(workers);

		std::vector<double> totals;
		std::vector<std::int64_t> wheres;
		fold(loop_schedule(pool, 1), totals, wheres);

		INFO( "workers " << workers );
		CHECK( totals == serial_totals );
		CHECK( wheres == serial_wheres );
	}
}

TEST_CASE(
	"run_reduction_loop should split the fold when there are too few outputs "
	"to share out",
	"[reduction_loop]"
)
{
	// One output and a deep fold behind it: the surviving space has nothing
	// to give a second thread, so this is the shape that has to reach the
	// merge to be threaded at all.
	//
	// Deep enough to be worth threading at the default grain, so that the
	// fold split is the path actually taken here rather than something the
	// case only describes.
	REXLIB_CONST_CONSTEXPR std::size_t reduced = 4*REXLIB_PARALLEL_GRAIN_SIZE;

	// Whole numbers, so that the sum is exact whatever order it is added in
	// and the check can be an equality rather than a tolerance. That the fold
	// reassociates is documented; what is pinned here is that it still visits
	// every element exactly once.
	//
	// The largest value appears twice, in the middle and at the very end.
	// Both are past the first third of the range, so neither falls in the
	// first slice however few slices there are, and they fall in different
	// slices however many. That is what makes the merge order observable:
	// the two are equal, so which position survives is decided by which
	// partial the merge sees first, and only merging them in the order the
	// slices sit in keeps the earlier one.
	std::vector<double> input(reduced, 0.0);
	input[reduced/2] = 1.0;
	input[reduced - 1] = 1.0;

	const auto kept_layout = make_layout({}, { {}, {}, {} });
	const auto reduced_layout = make_layout({ reduced }, { { 1 } });

	const auto fold =
		[&] (const loop_schedule &schedule, double &total, std::int64_t &where)
		{
			total = 0.0;
			where = -1;
			run_reduction_loop(
				sum_and_argmax_kernel(),
				kept_layout,
				reduced_layout,
				reduced,
				std::make_tuple(&total, &where),
				std::make_tuple(
					static_cast<const double*>(input.data())
				),
				schedule
			);
		};

	double serial_total = 0.0;
	std::int64_t serial_where = -1;
	fold(loop_schedule(), serial_total, serial_where);

	CHECK( serial_total == 2.0 );
	CHECK( serial_where == static_cast<std::int64_t>(reduced/2) );

	for (std::size_t workers : { std::size_t(1), std::size_t(3),
	                             std::size_t(4), std::size_t(8) })
	{
		thread_pool pool(workers);

		double total = 0.0;
		std::int64_t where = -1;
		fold(loop_schedule(pool, 1), total, where);

		INFO( "workers " << workers );
		CHECK( total == serial_total );

		// The first of the equal maxima, which only survives if the partials
		// are merged in the order the slices sit in rather than the order the
		// threads happened to finish, and if every slice numbered its
		// positions from the whole space rather than from itself.
		CHECK( where == serial_where );
	}
}

namespace
{

/**
 * @brief One invocation of a bulk member of a vector kernel.
 */
struct bulk_call
{
	const int *input;
	std::ptrdiff_t stride;
	std::size_t extent;
	std::size_t position;
	std::ptrdiff_t reduced_stride = 0;
	std::size_t count = 0;
};

/**
 * @brief Summing vector kernel that records the bulk calls it receives.
 *
 * Which of `combine_run` and `combine_strip` carries the fold is decided by
 * the layouts, so a kernel that records both is what tells them apart.
 */
class recording_vector_kernel
{
public:
	recording_vector_kernel()
		: m_runs(std::make_shared<std::vector<bulk_call>>())
		, m_strips(std::make_shared<std::vector<bulk_call>>())
	{
	}

	template <typename Outputs, typename Inputs>
	struct accumulators
	{
		using type = type_list<int>;
	};

	void seed(int &accumulator, const int *value, std::size_t) const
	{
		accumulator = *value;
	}

	template <typename... Strides>
	void combine_run(
		const std::tuple<int*> &accumulators,
		const std::tuple<const int*> &inputs,
		const std::tuple<Strides...> &strides,
		std::size_t count,
		std::size_t position
	) const
	{
		const auto stride = read_stride(strides);
		const auto *input = std::get<0>(inputs);
		m_runs->push_back(bulk_call{ input, stride, count, position });

		auto &accumulator = *std::get<0>(accumulators);
		for (std::size_t e = 0; e < count; ++e)
		{
			accumulator += input[static_cast<std::ptrdiff_t>(e)*stride];
		}
	}

	template <typename... KeptStrides, typename... ReducedStrides>
	void combine_strip(
		const std::tuple<int*> &accumulators,
		const std::tuple<const int*> &inputs,
		const std::tuple<KeptStrides...> &kept_strides,
		const std::tuple<ReducedStrides...> &reduced_strides,
		std::size_t width,
		std::size_t count,
		std::size_t position
	) const
	{
		const auto kept = read_stride(kept_strides);
		const auto reduced = read_stride(reduced_strides);
		const auto *input = std::get<0>(inputs);
		m_strips->push_back(
			bulk_call{ input, kept, width, position, reduced, count });

		auto *accumulators_begin = std::get<0>(accumulators);
		for (std::size_t e = 0; e < count; ++e)
		{
			const auto *row = input + static_cast<std::ptrdiff_t>(e)*reduced;
			for (std::size_t j = 0; j < width; ++j)
			{
				accumulators_begin[j] +=
					row[static_cast<std::ptrdiff_t>(j)*kept];
			}
		}
	}

	void merge(int &accumulator, const int &other) const
	{
		accumulator += other;
	}

	void finalize(int *result, const int &accumulator, std::size_t) const
	{
		*result = accumulator;
	}

	const std::vector<bulk_call>& runs() const noexcept
	{
		return *m_runs;
	}

	const std::vector<bulk_call>& strips() const noexcept
	{
		return *m_strips;
	}

private:
	std::shared_ptr<std::vector<bulk_call>> m_runs;
	std::shared_ptr<std::vector<bulk_call>> m_strips;

	template <typename... Strides>
	static std::ptrdiff_t read_stride(const std::tuple<Strides...> &strides)
	{
		return static_cast<std::ptrdiff_t>(std::get<0>(strides));
	}
};

} // anonymous namespace

TEST_CASE(
	"run_reduction_vector_loop should hand the fold over a run when the "
	"reduced axis is the contiguous one",
	"[reduction_loop]"
)
{
	// A 4x3 matrix folded along its contiguous axis. Each output's elements
	// are one stretch of memory, so they reach the kernel as one run and the
	// accumulator stays put for the whole of it.
	const auto input = iota_vector(12);
	std::vector<int> output(4, -1);

	const recording_vector_kernel kernel;
	run_reduction_vector_loop(
		kernel,
		make_layout({4}, {{3}, {1}}),
		make_layout({3}, {{1}}),
		3,
		std::make_tuple(output.data()),
		std::make_tuple(static_cast<const int*>(input.data()))
	);

	CHECK( kernel.strips().empty() );
	REQUIRE( kernel.runs().size() == 4 );

	for (std::size_t j = 0; j < 4; ++j)
	{
		INFO( "output " << j );

		// The first element of the run seeded the accumulator, so the run
		// handed over starts at the second and is one shorter.
		CHECK( kernel.runs()[j].input == input.data() + 3*j + 1 );
		CHECK( kernel.runs()[j].stride == 1 );
		CHECK( kernel.runs()[j].extent == 2 );
		CHECK( kernel.runs()[j].position == 1 );
	}

	CHECK( output == std::vector<int>({3, 12, 21, 30}) );
}

TEST_CASE(
	"run_reduction_vector_loop should hand the fold over a strip when the "
	"surviving axis is the contiguous one",
	"[reduction_loop]"
)
{
	// The same matrix folded along its outer axis. Consecutive outputs are
	// now consecutive in memory, so one element of the reduced space feeds
	// the whole strip and the strip is what gets walked.
	const auto input = iota_vector(12);
	std::vector<int> output(3, -1);

	const recording_vector_kernel kernel;
	run_reduction_vector_loop(
		kernel,
		make_layout({3}, {{1}, {1}}),
		make_layout({4}, {{3}}),
		4,
		std::make_tuple(output.data()),
		std::make_tuple(static_cast<const int*>(input.data()))
	);

	CHECK( kernel.runs().empty() );
	REQUIRE( kernel.strips().size() == 1 );

	// One call for the whole run, both loops belonging to the kernel: the
	// three elements of the reduced space that were not the seed, across the
	// three outputs of the strip.
	CHECK( kernel.strips()[0].input == input.data() + 3 );
	CHECK( kernel.strips()[0].stride == 1 );
	CHECK( kernel.strips()[0].extent == 3 );
	CHECK( kernel.strips()[0].reduced_stride == 3 );
	CHECK( kernel.strips()[0].count == 3 );
	CHECK( kernel.strips()[0].position == 1 );

	CHECK( output == std::vector<int>({18, 22, 26}) );
}

TEST_CASE(
	"run_reduction_vector_loop should hand over a stride it could not resolve "
	"as itself",
	"[reduction_loop]"
)
{
	// Where a stride is one or zero the kernel is handed a tag, which is what
	// lets one written against explicit vector types specialize on it; the
	// two orientation cases above check that. Anything else arrives as the
	// runtime value it is, and the walk still follows it.
	//
	// Neither axis is contiguous here, which is also what settles the
	// orientation: the run is only worth handing over whole when it is the
	// stream that reading it in order makes it, so everything else folds a
	// strip at a time.
	const auto input = iota_vector(12);
	std::vector<int> output(2, -1);

	const recording_vector_kernel kernel;
	run_reduction_vector_loop(
		kernel,
		make_layout({2}, {{6}, {1}}),
		make_layout({3}, {{2}}),
		3,
		std::make_tuple(output.data()),
		std::make_tuple(static_cast<const int*>(input.data()))
	);

	CHECK( kernel.runs().empty() );
	REQUIRE( kernel.strips().size() == 1 );

	CHECK( kernel.strips()[0].stride == 6 );
	CHECK( kernel.strips()[0].reduced_stride == 2 );
	CHECK( kernel.strips()[0].input == input.data() + 2 );
	CHECK( kernel.strips()[0].count == 2 );

	CHECK( output == std::vector<int>({0 + 2 + 4, 6 + 8 + 10}) );
}

TEST_CASE(
	"run_reduction_vector_loop should not invoke the kernel for a fold over "
	"a single element",
	"[reduction_loop]"
)
{
	// The seed is the whole answer, so there is nothing left to hand over in
	// bulk and neither member is called.
	const auto input = iota_vector(3);
	std::vector<int> output(3, -1);

	const recording_vector_kernel kernel;
	run_reduction_vector_loop(
		kernel,
		make_layout({3}, {{1}, {1}}),
		make_layout({}, {{}}),
		1,
		std::make_tuple(output.data()),
		std::make_tuple(static_cast<const int*>(input.data()))
	);

	CHECK( kernel.runs().empty() );
	CHECK( kernel.strips().empty() );
	CHECK( output == input );
}
