// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/core/hardware/program.hpp>

using namespace rex;

namespace
{

class test_program final : public program
{
};

} // namespace

TEST_CASE(
	"program::get_scratch_requirements returns empty span by default",
	"[hardware]"
)
{
	test_program prog;
	CHECK( prog.get_scratch_requirements().empty() );
}
