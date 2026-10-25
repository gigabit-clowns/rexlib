// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/program_builder_registration.hpp>

#include "mock/mock_program_builder.hpp"
#include "mock/mock_program_builder_registry.hpp"

#include <trompeloeil.hpp>

using namespace rexlib;

TEST_CASE(
	"program_builder_registration appends a factory for its builder to a "
	"registry",
	"[program_builder_registration]"
)
{
	mock_program_builder_registry registry;

	REQUIRE_CALL(registry, add(trompeloeil::_))
		.WITH(
			dynamic_cast<const mock_program_builder*>(_1().get()) != nullptr
		);

	const program_builder_registration<
		mock_program_builder,
		mock_program_builder_registry
	> registration(registry);
}
