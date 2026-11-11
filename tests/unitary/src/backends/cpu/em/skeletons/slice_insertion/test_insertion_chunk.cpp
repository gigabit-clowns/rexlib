// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/em/skeletons/slice_insertion/insertion_chunk.hpp>

using namespace rexlib::cpu;

TEST_CASE(
	"insertion_chunk should hold one array of its size per quantity",
	"[insertion_chunk]"
)
{
	insertion_chunk<float> chunk;

	STATIC_CHECK( insertion_chunk<float>::size == 64 );
	CHECK( chunk.get_coordinates(2).size() == 64 );
	CHECK( chunk.get_first_voxels(0).size() == 64 );
	CHECK( &chunk.get_coordinates(0) != &chunk.get_coordinates(1) );
	CHECK( &chunk.get_first_voxels(1) != &chunk.get_first_voxels(2) );
}

TEST_CASE(
	"insertion_chunk should keep what is written into it",
	"[insertion_chunk]"
)
{
	insertion_chunk<double> chunk;
	chunk.get_coordinates(1)[3] = 2.5;
	chunk.get_first_voxels(2)[4] = -7;
	chunk.get_numerator_weights()[5] = 1.5;
	chunk.get_denominator_weights()[6] = 2.0;
	chunk.get_radial_weights()[7] = 0.5;
	chunk.get_values_real()[8] = -1.0;
	chunk.get_values_imaginary()[9] = 3.0;
	chunk.get_counts()[10] = 4.0;

	const auto &view = chunk;
	CHECK( view.get_coordinates(1)[3] == 2.5 );
	CHECK( view.get_first_voxels(2)[4] == -7 );
	CHECK( view.get_numerator_weights()[5] == 1.5 );
	CHECK( view.get_denominator_weights()[6] == 2.0 );
	CHECK( view.get_radial_weights()[7] == 0.5 );
	CHECK( view.get_values_real()[8] == -1.0 );
	CHECK( view.get_values_imaginary()[9] == 3.0 );
	CHECK( view.get_counts()[10] == 4.0 );
}
