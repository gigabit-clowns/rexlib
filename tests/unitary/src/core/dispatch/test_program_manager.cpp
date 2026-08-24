// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/program_manager.hpp>
#include <rexlib/core/dispatch/program_cache.hpp>

#include <rexlib/core/exceptions/invalid_operation_error.hpp>

#include "mock/mock_operation.hpp"
#include "mock/mock_program_builder.hpp"
#include "../hardware/mock/mock_program.hpp"
#include "../hardware/mock/mock_command_queue.hpp"

#include <array>

using namespace rexlib;

namespace
{

struct mock_operation_a
	: mock_operation
{
	~mock_operation_a() override = default;
};

struct mock_operation_b
	: mock_operation
{
	~mock_operation_b() override = default;
};

} // namespace

TEST_CASE(
	"program_manager::register_builder accepts a valid builder",
	"[program_manager]"
)
{
	program_manager manager;
	auto builder = std::make_unique<mock_program_builder>();

	REQUIRE_CALL(*builder, get_operation_id())
		.RETURN(operation_id::of<mock_program_builder>());

	CHECK(manager.register_builder(std::move(builder)));
}

TEST_CASE(
	"program_manager::register_builder rejects a null builder",
	"[program_manager]"
)
{
	program_manager manager;
	CHECK_FALSE(manager.register_builder(nullptr));
}

TEST_CASE(
	"program_manager::register_builder accepts multiple builders "
	"for the same operation",
	"[program_manager]"
)
{
	program_manager manager;
	auto builder1 = std::make_unique<mock_program_builder>();
	auto builder2 = std::make_unique<mock_program_builder>();

	REQUIRE_CALL(*builder1, get_operation_id())
		.RETURN(operation_id::of<mock_program_builder>());
	REQUIRE_CALL(*builder2, get_operation_id())
		.RETURN(operation_id::of<mock_program_builder>());

	CHECK(manager.register_builder(std::move(builder1)));
	CHECK(manager.register_builder(std::move(builder2)));
}

TEST_CASE(
	"program_manager::build dispatches to the builder with the "
	"highest suitability",
	"[program_manager]"
)
{
	program_manager manager;

	const mock_operation_a op;
	const std::array<operand_signature, 2> output_signatures;
	const std::array<operand_signature, 4> input_signatures;
	mock_command_queue queue;

	auto builder1 = std::make_unique<mock_program_builder>();
	auto builder2 = std::make_unique<mock_program_builder>();
	const auto prog = std::make_shared<mock_program>();

	REQUIRE_CALL(*builder1, get_operation_id())
		.RETURN(operation_id::of<mock_operation_a>());
	REQUIRE_CALL(
		*builder1,
		get_suitability(
			ANY(const operation&),
			ANY(span<const operand_signature>),
			ANY(span<const operand_signature>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::fallback);

	REQUIRE_CALL(*builder2, get_operation_id())
		.RETURN(operation_id::of<mock_operation_a>());
	REQUIRE_CALL(
		*builder2,
		get_suitability(
			ANY(const operation&),
			ANY(span<const operand_signature>),
			ANY(span<const operand_signature>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::normal);
	REQUIRE_CALL(
		*builder2,
		build(
			ANY(const operation&),
			ANY(span<const operand_signature>),
			ANY(span<const operand_signature>),
			ANY(command_queue&),
			ANY(program_cache*)
		)
	)
		.RETURN(prog);

	REQUIRE(manager.register_builder(std::move(builder1)));
	REQUIRE(manager.register_builder(std::move(builder2)));

	CHECK(
		manager.build(
			op,
			make_span(output_signatures),
			make_span(input_signatures),
			queue
		) == prog
	);
}

TEST_CASE(
	"program_manager::build filters builders by operation id",
	"[program_manager]"
)
{
	program_manager manager;

	const mock_operation_a op;
	const std::array<operand_signature, 2> output_signatures;
	const std::array<operand_signature, 4> input_signatures;
	mock_command_queue queue;

	auto builder_a = std::make_unique<mock_program_builder>();
	auto builder_b = std::make_unique<mock_program_builder>();
	const auto prog = std::make_shared<mock_program>();

	REQUIRE_CALL(*builder_a, get_operation_id())
		.RETURN(operation_id::of<mock_operation_a>());
	REQUIRE_CALL(
		*builder_a,
		get_suitability(
			ANY(const operation&),
			ANY(span<const operand_signature>),
			ANY(span<const operand_signature>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::normal);
	REQUIRE_CALL(
		*builder_a,
		build(
			ANY(const operation&),
			ANY(span<const operand_signature>),
			ANY(span<const operand_signature>),
			ANY(command_queue&),
			ANY(program_cache*)
		)
	)
		.RETURN(prog);

	// Registered for a different op id: must not be consulted.
	REQUIRE_CALL(*builder_b, get_operation_id())
		.RETURN(operation_id::of<mock_operation_b>());

	REQUIRE(manager.register_builder(std::move(builder_a)));
	REQUIRE(manager.register_builder(std::move(builder_b)));

	CHECK(
		manager.build(
			op,
			make_span(output_signatures),
			make_span(input_signatures),
			queue
		) == prog
	);
}

TEST_CASE(
	"program_manager::build forwards a null cache by default",
	"[program_manager]"
)
{
	program_manager manager;

	const mock_operation_a op;
	const std::array<operand_signature, 2> output_signatures;
	const std::array<operand_signature, 4> input_signatures;
	mock_command_queue queue;

	auto builder = std::make_unique<mock_program_builder>();
	const auto prog = std::make_shared<mock_program>();

	REQUIRE_CALL(*builder, get_operation_id())
		.RETURN(operation_id::of<mock_operation_a>());
	REQUIRE_CALL(
		*builder,
		get_suitability(
			ANY(const operation&),
			ANY(span<const operand_signature>),
			ANY(span<const operand_signature>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::normal);
	REQUIRE_CALL(
		*builder,
		build(
			ANY(const operation&),
			ANY(span<const operand_signature>),
			ANY(span<const operand_signature>),
			ANY(command_queue&),
			ANY(program_cache*)
		)
	)
		.LR_WITH(_5 == nullptr)
		.RETURN(prog);

	REQUIRE(manager.register_builder(std::move(builder)));

	CHECK(
		manager.build(
			op,
			make_span(output_signatures),
			make_span(input_signatures),
			queue
		) == prog
	);
}

TEST_CASE(
	"program_manager::build forwards the cache pointer to the "
	"selected builder",
	"[program_manager]"
)
{
	program_manager manager;

	const mock_operation_a op;
	const std::array<operand_signature, 2> output_signatures;
	const std::array<operand_signature, 4> input_signatures;
	program_cache cache(4);
	mock_command_queue queue;

	auto builder = std::make_unique<mock_program_builder>();
	const auto prog = std::make_shared<mock_program>();

	REQUIRE_CALL(*builder, get_operation_id())
		.RETURN(operation_id::of<mock_operation_a>());
	REQUIRE_CALL(
		*builder,
		get_suitability(
			ANY(const operation&),
			ANY(span<const operand_signature>),
			ANY(span<const operand_signature>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::normal);
	REQUIRE_CALL(
		*builder,
		build(
			ANY(const operation&),
			ANY(span<const operand_signature>),
			ANY(span<const operand_signature>),
			ANY(command_queue&),
			ANY(program_cache*)
		)
	)
		.LR_WITH(_5 == &cache)
		.RETURN(prog);

	REQUIRE(manager.register_builder(std::move(builder)));

	CHECK(
		manager.build(
			op,
			make_span(output_signatures),
			make_span(input_signatures),
			queue,
			&cache
		) == prog
	);
}

TEST_CASE(
	"program_manager::build throws when no builder handles the "
	"operation",
	"[program_manager]"
)
{
	program_manager manager;

	const mock_operation_b op;
	const std::array<operand_signature, 2> output_signatures;
	const std::array<operand_signature, 4> input_signatures;
	mock_command_queue queue;

	auto builder = std::make_unique<mock_program_builder>();
	REQUIRE_CALL(*builder, get_operation_id())
		.RETURN(operation_id::of<mock_operation_a>());

	REQUIRE(manager.register_builder(std::move(builder)));

	REQUIRE_THROWS_AS(
		manager.build(
			op,
			make_span(output_signatures),
			make_span(input_signatures),
			queue
		),
		invalid_operation_error
	);
}

TEST_CASE(
	"program_manager::build throws when every candidate reports "
	"unsupported",
	"[program_manager]"
)
{
	program_manager manager;

	const mock_operation_a op;
	const std::array<operand_signature, 2> output_signatures;
	const std::array<operand_signature, 4> input_signatures;
	mock_command_queue queue;

	auto builder1 = std::make_unique<mock_program_builder>();
	auto builder2 = std::make_unique<mock_program_builder>();

	REQUIRE_CALL(*builder1, get_operation_id())
		.RETURN(operation_id::of<mock_operation_a>());
	REQUIRE_CALL(
		*builder1,
		get_suitability(
			ANY(const operation&),
			ANY(span<const operand_signature>),
			ANY(span<const operand_signature>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::unsupported);

	REQUIRE_CALL(*builder2, get_operation_id())
		.RETURN(operation_id::of<mock_operation_a>());
	REQUIRE_CALL(
		*builder2,
		get_suitability(
			ANY(const operation&),
			ANY(span<const operand_signature>),
			ANY(span<const operand_signature>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::unsupported);

	REQUIRE(manager.register_builder(std::move(builder1)));
	REQUIRE(manager.register_builder(std::move(builder2)));

	REQUIRE_THROWS_AS(
		manager.build(
			op,
			make_span(output_signatures),
			make_span(input_signatures),
			queue
		),
		invalid_operation_error
	);
}
