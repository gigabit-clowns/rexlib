// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/linalg_program_builder.hpp>

#include <rexlib/ops/linalg/matmul_operation.hpp>
#include <rexlib/ops/linalg/matvec_operation.hpp>
#include <rexlib/ops/linalg/vecmat_operation.hpp>

using namespace rexlib;
using namespace rexlib::cpu;

TEST_CASE(
	"linalg_core_ranks should read the core ranks of a matrix product from "
	"its shape rule",
	"[linalg_program_builder]"
)
{
	using ranks = linalg_core_ranks<ops::matmul_operation>;
	STATIC_CHECK( ranks::output == 2 );
	STATIC_CHECK( ranks::left == 2 );
	STATIC_CHECK( ranks::right == 2 );
}

TEST_CASE(
	"linalg_core_ranks should read the core ranks of a matrix-vector "
	"product from its shape rule",
	"[linalg_program_builder]"
)
{
	using ranks = linalg_core_ranks<ops::matvec_operation>;
	STATIC_CHECK( ranks::output == 1 );
	STATIC_CHECK( ranks::left == 2 );
	STATIC_CHECK( ranks::right == 1 );
}

TEST_CASE(
	"linalg_core_ranks should read the core ranks of a vector-matrix "
	"product from its shape rule",
	"[linalg_program_builder]"
)
{
	using ranks = linalg_core_ranks<ops::vecmat_operation>;
	STATIC_CHECK( ranks::output == 1 );
	STATIC_CHECK( ranks::left == 1 );
	STATIC_CHECK( ranks::right == 2 );
}
