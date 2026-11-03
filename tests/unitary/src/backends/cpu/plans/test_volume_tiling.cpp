// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/plans/volume_tiling.hpp>

using namespace rexlib::cpu;

namespace
{

const std::size_t megabyte = 1024 * 1024;

} // anonymous namespace

TEST_CASE(
	"choose_tile_extent should take the largest power of two within the "
	"budget that gives enough tiles",
	"[volume_tiling]"
)
{
	// 257 voxels of 12 bytes per stored row.
	CHECK( choose_tile_extent(512, 257 * 12, megabyte, 128, 1.0) == 16 );
	CHECK( choose_tile_extent(256, 129 * 12, megabyte, 128, 1.0) == 16 );
	CHECK( choose_tile_extent(256, 129 * 12, megabyte, 16, 1.0) == 16 );
	CHECK( choose_tile_extent(256, 129 * 12, 4 * megabyte, 16, 1.0) == 32 );
}

TEST_CASE(
	"choose_tile_extent should not go below eight half-widths",
	"[volume_tiling]"
)
{
	CHECK( choose_tile_extent(64, 33 * 12, megabyte, 128, 1.0) == 8 );
	CHECK( choose_tile_extent(64, 33 * 12, megabyte, 128, 2.0) == 16 );
	CHECK( choose_tile_extent(64, 33 * 12, megabyte, 128, 0.5) == 4 );
}

TEST_CASE(
	"choose_tile_extent should never exceed the extent",
	"[volume_tiling]"
)
{
	CHECK( choose_tile_extent(4, 3 * 12, megabyte, 1, 2.0) == 4 );
	CHECK( choose_tile_extent(1, 12, megabyte, 1, 1.0) == 1 );
	CHECK( choose_tile_extent(6, 4 * 12, megabyte, 1, 0.5) == 4 );
}

TEST_CASE(
	"volume_tiling should cover the volume with tiles, the last ones cut",
	"[volume_tiling]"
)
{
	const volume_tiling tiling(10, 4);

	CHECK( tiling.get_tile_count() == 9 );

	const auto first = tiling.get_tile(0);
	CHECK( first.get_z_begin() == 0 );
	CHECK( first.get_z_end() == 4 );
	CHECK( first.get_y_begin() == 0 );
	CHECK( first.get_y_end() == 4 );

	const auto second = tiling.get_tile(1);
	CHECK( second.get_z_begin() == 0 );
	CHECK( second.get_y_begin() == 4 );

	const auto last = tiling.get_tile(8);
	CHECK( last.get_extent() == 10 );
	CHECK( last.get_z_begin() == 8 );
	CHECK( last.get_z_end() == 10 );
	CHECK( last.get_y_begin() == 8 );
	CHECK( last.get_y_end() == 10 );
}
