// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/program_builder_registry.hpp>
#include <rexlib/core/dispatch/program_service.hpp>

#include <rexlib/core/backend_priority.hpp>
#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/operation/operation_id.hpp>

#include "mock/mock_program_builder.hpp"
#include "mock/mock_program_builder_factory.hpp"
#include "../hardware/mock/mock_command_queue.hpp"
#include "../hardware/mock/mock_program.hpp"
#include "../operation/mock/mock_operation.hpp"

#include <array>
#include <memory>
#include <trompeloeil.hpp>
#include <utility>

using namespace rexlib;

namespace
{

std::shared_ptr<program> build_mock_operation(
	const program_service &programs,
	command_queue &queue
)
{
	const mock_operation op;
	const std::array<operand_descriptor, 1> output_descriptors;
	const std::array<operand_descriptor, 0> input_descriptors;

	return programs.build(
		op,
		make_span(output_descriptors),
		make_span(input_descriptors),
		queue
	);
}

} // anonymous namespace

TEST_CASE(
	"program_builder_registry::register_all hands a program service what its "
	"factories make",
	"[program_builder_registry]"
)
{
	using factory = mock_program_builder_factory;
	using trompeloeil::_;

	program_builder_registry registry;
	mock_command_queue queue;
	// Declared before the expectations, so the builder it owns outlives them.
	program_service programs;

	auto builder = std::make_unique<mock_program_builder>();
	const auto prog = std::make_shared<mock_program>();

	REQUIRE_CALL(*builder, get_operation_id())
		.RETURN(operation_id::of<mock_operation>());
	REQUIRE_CALL(*builder, get_suitability(_, _, _, _))
		.LR_WITH(&_4 == &queue)
		.RETURN(backend_priority::normal);
	REQUIRE_CALL(*builder, build(_, _, _, _, _))
		.LR_WITH(&_4 == &queue)
		.RETURN(prog);
	REQUIRE_CALL(factory::get_instance(), make())
		.LR_RETURN(std::move(builder));

	registry.add(&factory::create);
	registry.register_all(programs);

	CHECK( build_mock_operation(programs, queue) == prog );
}

TEST_CASE(
	"program_builder_registry::register_all on an empty registry registers "
	"nothing",
	"[program_builder_registry]"
)
{
	program_builder_registry registry;
	mock_command_queue queue;
	program_service programs;

	registry.register_all(programs);

	CHECK_THROWS_AS(
		build_mock_operation(programs, queue),
		unsupported_operation_error
	);
}

TEST_CASE(
	"program_builder_registry::add ignores null factories",
	"[program_builder_registry]"
)
{
	program_builder_registry registry;
	mock_command_queue queue;
	program_service programs;

	registry.add(nullptr);
	registry.register_all(programs);

	CHECK_THROWS_AS(
		build_mock_operation(programs, queue),
		unsupported_operation_error
	);
}
