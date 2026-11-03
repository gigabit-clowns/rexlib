// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/program_cache.hpp>
#include <rexlib/core/dispatch/program_service.hpp>

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>

#include "../operation/mock/mock_operation.hpp"
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

} // anonymous namespace

TEST_CASE(
	"program_service::register_builder accepts a valid builder",
	"[program_service]"
)
{
	program_service programs;
	auto builder = std::make_unique<mock_program_builder>();

	REQUIRE_CALL(*builder, get_operation_id())
		.RETURN(operation_id::of<mock_program_builder>());

	CHECK(programs.register_builder(std::move(builder)));
}

TEST_CASE(
	"program_service::register_builder rejects a null builder",
	"[program_service]"
)
{
	program_service programs;
	CHECK_FALSE(programs.register_builder(nullptr));
}

TEST_CASE(
	"program_service::register_builder accepts multiple builders "
	"for the same operation",
	"[program_service]"
)
{
	program_service programs;
	auto builder1 = std::make_unique<mock_program_builder>();
	auto builder2 = std::make_unique<mock_program_builder>();

	REQUIRE_CALL(*builder1, get_operation_id())
		.RETURN(operation_id::of<mock_program_builder>());
	REQUIRE_CALL(*builder2, get_operation_id())
		.RETURN(operation_id::of<mock_program_builder>());

	CHECK(programs.register_builder(std::move(builder1)));
	CHECK(programs.register_builder(std::move(builder2)));
}

TEST_CASE(
	"program_service::build dispatches to the builder with the "
	"highest suitability",
	"[program_service]"
)
{
	program_service programs;

	const mock_operation_a op;
	const std::array<operand_descriptor, 2> output_descriptors;
	const std::array<operand_descriptor, 4> input_descriptors;
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
			ANY(span<const operand_descriptor>),
			ANY(span<const operand_descriptor>),
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
			ANY(span<const operand_descriptor>),
			ANY(span<const operand_descriptor>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::normal);
	REQUIRE_CALL(
		*builder2,
		build(
			ANY(const operation&),
			ANY(span<const operand_descriptor>),
			ANY(span<const operand_descriptor>),
			ANY(command_queue&),
			ANY(program_cache*)
		)
	)
		.RETURN(prog);

	REQUIRE(programs.register_builder(std::move(builder1)));
	REQUIRE(programs.register_builder(std::move(builder2)));

	CHECK(
		programs.build(
			op,
			make_span(output_descriptors),
			make_span(input_descriptors),
			queue
		) == prog
	);
}

TEST_CASE(
	"program_service::build filters builders by operation id",
	"[program_service]"
)
{
	program_service programs;

	const mock_operation_a op;
	const std::array<operand_descriptor, 2> output_descriptors;
	const std::array<operand_descriptor, 4> input_descriptors;
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
			ANY(span<const operand_descriptor>),
			ANY(span<const operand_descriptor>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::normal);
	REQUIRE_CALL(
		*builder_a,
		build(
			ANY(const operation&),
			ANY(span<const operand_descriptor>),
			ANY(span<const operand_descriptor>),
			ANY(command_queue&),
			ANY(program_cache*)
		)
	)
		.RETURN(prog);

	// Registered for a different op id: must not be consulted.
	REQUIRE_CALL(*builder_b, get_operation_id())
		.RETURN(operation_id::of<mock_operation_b>());

	REQUIRE(programs.register_builder(std::move(builder_a)));
	REQUIRE(programs.register_builder(std::move(builder_b)));

	CHECK(
		programs.build(
			op,
			make_span(output_descriptors),
			make_span(input_descriptors),
			queue
		) == prog
	);
}

TEST_CASE(
	"program_service::build forwards a null cache by default",
	"[program_service]"
)
{
	program_service programs;

	const mock_operation_a op;
	const std::array<operand_descriptor, 2> output_descriptors;
	const std::array<operand_descriptor, 4> input_descriptors;
	mock_command_queue queue;

	auto builder = std::make_unique<mock_program_builder>();
	const auto prog = std::make_shared<mock_program>();

	REQUIRE_CALL(*builder, get_operation_id())
		.RETURN(operation_id::of<mock_operation_a>());
	REQUIRE_CALL(
		*builder,
		get_suitability(
			ANY(const operation&),
			ANY(span<const operand_descriptor>),
			ANY(span<const operand_descriptor>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::normal);
	REQUIRE_CALL(
		*builder,
		build(
			ANY(const operation&),
			ANY(span<const operand_descriptor>),
			ANY(span<const operand_descriptor>),
			ANY(command_queue&),
			ANY(program_cache*)
		)
	)
		.LR_WITH(_5 == nullptr)
		.RETURN(prog);

	REQUIRE(programs.register_builder(std::move(builder)));

	CHECK(
		programs.build(
			op,
			make_span(output_descriptors),
			make_span(input_descriptors),
			queue
		) == prog
	);
}

TEST_CASE(
	"program_service::build forwards the cache pointer to the "
	"selected builder",
	"[program_service]"
)
{
	program_service programs;

	const mock_operation_a op;
	const std::array<operand_descriptor, 2> output_descriptors;
	const std::array<operand_descriptor, 4> input_descriptors;
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
			ANY(span<const operand_descriptor>),
			ANY(span<const operand_descriptor>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::normal);
	REQUIRE_CALL(
		*builder,
		build(
			ANY(const operation&),
			ANY(span<const operand_descriptor>),
			ANY(span<const operand_descriptor>),
			ANY(command_queue&),
			ANY(program_cache*)
		)
	)
		.LR_WITH(_5 == &cache)
		.RETURN(prog);

	REQUIRE(programs.register_builder(std::move(builder)));

	CHECK(
		programs.build(
			op,
			make_span(output_descriptors),
			make_span(input_descriptors),
			queue,
			&cache
		) == prog
	);
}

TEST_CASE(
	"program_service::build throws when no builder handles the "
	"operation",
	"[program_service]"
)
{
	program_service programs;

	const mock_operation_b op;
	const std::array<operand_descriptor, 2> output_descriptors;
	const std::array<operand_descriptor, 4> input_descriptors;
	mock_command_queue queue;

	auto builder = std::make_unique<mock_program_builder>();
	REQUIRE_CALL(*builder, get_operation_id())
		.RETURN(operation_id::of<mock_operation_a>());

	REQUIRE(programs.register_builder(std::move(builder)));

	REQUIRE_THROWS_AS(
		programs.build(
			op,
			make_span(output_descriptors),
			make_span(input_descriptors),
			queue
		),
		unsupported_operation_error
	);
}

TEST_CASE(
	"program_service::build throws when every candidate reports "
	"unsupported",
	"[program_service]"
)
{
	program_service programs;

	const mock_operation_a op;
	const std::array<operand_descriptor, 2> output_descriptors;
	const std::array<operand_descriptor, 4> input_descriptors;
	mock_command_queue queue;

	auto builder1 = std::make_unique<mock_program_builder>();
	auto builder2 = std::make_unique<mock_program_builder>();

	REQUIRE_CALL(*builder1, get_operation_id())
		.RETURN(operation_id::of<mock_operation_a>());
	REQUIRE_CALL(
		*builder1,
		get_suitability(
			ANY(const operation&),
			ANY(span<const operand_descriptor>),
			ANY(span<const operand_descriptor>),
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
			ANY(span<const operand_descriptor>),
			ANY(span<const operand_descriptor>),
			ANY(command_queue&)
		)
	)
		.RETURN(backend_priority::unsupported);

	REQUIRE(programs.register_builder(std::move(builder1)));
	REQUIRE(programs.register_builder(std::move(builder2)));

	REQUIRE_THROWS_AS(
		programs.build(
			op,
			make_span(output_descriptors),
			make_span(input_descriptors),
			queue
		),
		unsupported_operation_error
	);
}
