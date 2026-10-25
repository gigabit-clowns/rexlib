// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/hardware/program.hpp>

using namespace rexlib;

TEST_CASE(
	"program::get_scratch_requirements returns empty span by default",
	"[hardware]"
)
{
	const program prog;
	CHECK( prog.get_scratch_requirements().empty() );
}
