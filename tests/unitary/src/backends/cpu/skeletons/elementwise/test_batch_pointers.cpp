// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/skeletons/elementwise/batch_pointers.hpp>

#include <cstddef>
#include <tuple>
#include <utility>

using namespace rexlib;
using namespace rexlib::cpu;

TEST_CASE(
	"slice_pointers should take a run of the pointers",
	"[batch_pointers]"
)
{
	int a = 0;
	int b = 0;
	int c = 0;
	const auto pointers = std::make_tuple(&a, &b, &c);

	CHECK(
		slice_pointers<1>(pointers, std::make_index_sequence<2>()) ==
		std::make_tuple(&b, &c)
	);
}

TEST_CASE(
	"get_batch_counter should read the last of the pointers for a linear "
	"index, and 0 otherwise",
	"[batch_pointers]"
)
{
	int a = 0;
	const auto pointers = std::make_tuple(&a, std::size_t(7));

	CHECK( get_batch_counter(pointers, linear_index_tag()) == 7 );
	CHECK( get_batch_counter(pointers, no_index_tag()) == 0 );
	CHECK( get_batch_counter(pointers, multidimensional_index_tag()) == 0 );
}
