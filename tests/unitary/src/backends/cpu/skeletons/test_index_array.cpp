// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/skeletons/index_array.hpp>

#include <array>
#include <cstddef>
#include <utility>

using namespace rexlib;
using namespace rexlib::cpu;

TEST_CASE(
	"make_index_array should hold the indices of the sequence in order",
	"[index_array]"
)
{
	const auto indices = make_index_array(std::index_sequence<0, 3, 4>());
	CHECK( indices == std::array<std::size_t, 3>{{ 0, 3, 4 }} );
	CHECK( make_index_array(std::index_sequence<>()).empty() );
}
