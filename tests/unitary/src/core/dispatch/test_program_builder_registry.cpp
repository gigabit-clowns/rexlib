// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/program_builder_registry.hpp>
#include <rexlib/core/dispatch/program_manager.hpp>

#include <rexlib/core/exceptions/invalid_operation_error.hpp>

#include "fixtures/stub_program_builder.hpp"

using namespace rexlib;
using namespace rexlib::test;

TEST_CASE(
	"program_builder_registry::register_all registers every factory's "
	"builder into a manager",
	"[program_builder_registry]"
)
{
	program_builder_registry registry;
	registry.add(&make_stub_builder);

	program_manager manager;
	registry.register_all(manager);

	CHECK( build_stub_operation(manager) != nullptr );
}

TEST_CASE(
	"program_builder_registry::register_all on an empty registry registers "
	"nothing",
	"[program_builder_registry]"
)
{
	program_builder_registry registry;

	program_manager manager;
	registry.register_all(manager);

	CHECK_THROWS_AS(
		build_stub_operation(manager),
		invalid_operation_error
	);
}

TEST_CASE(
	"program_builder_registry::add ignores null factories",
	"[program_builder_registry]"
)
{
	program_builder_registry registry;
	registry.add(nullptr);

	program_manager manager;
	registry.register_all(manager);

	CHECK_THROWS_AS(
		build_stub_operation(manager),
		invalid_operation_error
	);
}
