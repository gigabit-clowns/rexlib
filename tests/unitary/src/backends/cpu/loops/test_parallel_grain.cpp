// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/loops/parallel_grain.hpp>

#include <algorithm>
#include <cstddef>

using namespace rexlib::cpu;

TEST_CASE(
	"grain_for_cost should divide the grain by the cost of an iteration",
	"[parallel_grain]"
)
{
	const auto grain = get_parallel_grain_size();

	CHECK( grain > 0 );
	CHECK( grain_for_cost(0) == grain );
	CHECK( grain_for_cost(1) == grain );
	CHECK( grain_for_cost(2) == (grain / 2 > 0 ? grain / 2 : 1) );
	CHECK( grain_for_cost(grain * 4) == 1 );
}

TEST_CASE(
	"should_split_inner_space should prefer the outer space while it has "
	"the parallelism to spare",
	"[parallel_grain]"
)
{
	const auto inner = std::max<std::size_t>(get_parallel_grain_size(), 8);

	CHECK_FALSE( should_split_inner_space(8, inner, 4) );
	CHECK_FALSE( should_split_inner_space(100, inner, 4) );
}

TEST_CASE(
	"should_split_inner_space should split the inner space of too few "
	"outer iterations",
	"[parallel_grain]"
)
{
	const auto inner = std::max<std::size_t>(get_parallel_grain_size(), 8);

	CHECK( should_split_inner_space(1, inner, 4) );
	CHECK( should_split_inner_space(7, inner, 4) );
}

TEST_CASE(
	"should_split_inner_space should split nothing without workers, "
	"iterations or a grain of work",
	"[parallel_grain]"
)
{
	const auto grain = get_parallel_grain_size();
	const auto inner = std::max<std::size_t>(grain, 8);

	CHECK_FALSE( should_split_inner_space(1, inner, 1) );
	CHECK_FALSE( should_split_inner_space(0, inner, 4) );
	CHECK_FALSE( should_split_inner_space(1, 7, 4) );
	if (grain > 8)
	{
		CHECK_FALSE( should_split_inner_space(1, grain - 1, 4) );
	}
}
