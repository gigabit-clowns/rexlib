// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/program_builder_registration.hpp>

#include <rexlib/core/dispatch/program_builder_registry.hpp>
#include <rexlib/core/dispatch/program_manager.hpp>

#include "fixtures/stub_program_builder.hpp"

using namespace rexlib;
using namespace rexlib::test;

TEST_CASE(
	"program_builder_registration appends its builder factory to a registry",
	"[program_builder_registration]"
)
{
	program_builder_registry registry;
	const program_builder_registration<stub_program_builder> registration(
		registry
	);

	program_manager manager;
	registry.register_all(manager);

	CHECK( build_stub_operation(manager) != nullptr );
}
