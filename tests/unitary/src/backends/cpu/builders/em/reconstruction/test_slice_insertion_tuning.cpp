// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/em/reconstruction/slice_insertion_tuning.hpp>

using namespace rexlib::cpu;

TEST_CASE(
	"slice_insertion_tuning should default to 1 MiB tiles, 8 per thread",
	"[slice_insertion_tuning]"
)
{
	const slice_insertion_tuning tuning;

	CHECK( tuning.get_tile_byte_budget() == 1 << 20 );
	CHECK( tuning.get_tiles_per_thread() == 8 );
}

TEST_CASE(
	"slice_insertion_tuning should keep the values it was given",
	"[slice_insertion_tuning]"
)
{
	const slice_insertion_tuning tuning(4096, 2);

	CHECK( tuning.get_tile_byte_budget() == 4096 );
	CHECK( tuning.get_tiles_per_thread() == 2 );
}
