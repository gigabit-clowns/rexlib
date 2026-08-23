// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/plans/linalg_operand_core.hpp>

using namespace rex::cpu;

TEST_CASE(
	"linalg_operand_core default constructs as an empty core",
	"[linalg_operand_core]"
)
{
	const linalg_operand_core core;

	CHECK( core.get_rank() == 0 );
	CHECK( core.get_extent(0) == 0 );
	CHECK( core.get_extent(1) == 0 );
	CHECK( core.get_stride(0) == 0 );
	CHECK( core.get_stride(1) == 0 );
}

TEST_CASE(
	"linalg_operand_core stores a rank two core as given",
	"[linalg_operand_core]"
)
{
	const linalg_operand_core core(2, { 3, 4 }, { 4, 1 });

	CHECK( core.get_rank() == 2 );
	CHECK( core.get_extent(0) == 3 );
	CHECK( core.get_extent(1) == 4 );
	CHECK( core.get_stride(0) == 4 );
	CHECK( core.get_stride(1) == 1 );
}

TEST_CASE(
	"linalg_operand_core stores a rank one core as given",
	"[linalg_operand_core]"
)
{
	const linalg_operand_core core(1, { 5, 0 }, { 2, 0 });

	CHECK( core.get_rank() == 1 );
	CHECK( core.get_extent(0) == 5 );
	CHECK( core.get_stride(0) == 2 );
}
