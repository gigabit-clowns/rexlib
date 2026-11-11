// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/dispatch/cpu_program_builder_registry.hpp>

using namespace rexlib;
using namespace rexlib::cpu;

TEST_CASE(
	"get_cpu_program_builder_registry returns the same registry every time",
	"[cpu_program_builder_registry]"
)
{
	CHECK(
		&get_cpu_program_builder_registry() ==
		&get_cpu_program_builder_registry()
	);
}
