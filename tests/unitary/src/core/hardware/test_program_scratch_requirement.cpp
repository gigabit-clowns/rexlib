// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rex/core/hardware/program_scratch_requirement.hpp>

using namespace rex;

TEST_CASE(
	"program_scratch_requirement constructor should store size, alignment, "
	"and affinity",
	"[hardware]"
)
{
	const std::size_t size = GENERATE(2, 20, 1024);
	const std::size_t alignment = GENERATE(1, 4, 64);
	const auto affinity = GENERATE(
		memory_resource_affinity::device,
		memory_resource_affinity::host
	);

	program_scratch_requirement req(size, alignment, affinity);

	CHECK(req.get_size() == size);
	CHECK(req.get_alignment() == alignment);
	CHECK(req.get_affinity() == affinity);
}
