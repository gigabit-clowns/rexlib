// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/program_builder_registry.hpp>
#include <rexlib/core/dispatch/program_builder.hpp>
#include <rexlib/core/dispatch/program_manager.hpp>
#include <rexlib/core/dispatch/operation_id.hpp>
#include <rexlib/core/dispatch/operand_signature.hpp>

#include <rexlib/core/exceptions/invalid_operation_error.hpp>

#include "mock/mock_operation.hpp"
#include "../hardware/mock/mock_program.hpp"
#include "../hardware/mock/mock_command_queue.hpp"

#include <array>
#include <memory>

using namespace rexlib;

namespace
{

struct stub_operation
	: mock_operation
{
	~stub_operation() override = default;
};

// A concrete builder rather than a trompeloeil mock: the registry stores
// factories that create fresh instances on demand, which does not fit
// per-instance mock expectations. Only get_operation_id and build are
// exercised through the manager.
class stub_program_builder final
	: public program_builder
{
public:
	operation_id get_operation_id() const noexcept override
	{
		return operation_id::of<stub_operation>();
	}

	backend_priority get_suitability(
		const operation&,
		span<const operand_signature>,
		span<const operand_signature>,
		command_queue&
	) const override
	{
		return backend_priority::normal;
	}

	std::shared_ptr<program> build(
		const operation&,
		span<const operand_signature>,
		span<const operand_signature>,
		command_queue&,
		program_cache*
	) const override
	{
		return std::make_shared<mock_program>();
	}
};

std::unique_ptr<program_builder> make_stub_builder()
{
	return std::make_unique<stub_program_builder>();
}

std::shared_ptr<program> build_stub_operation(const program_manager &manager)
{
	const stub_operation op;
	const std::array<operand_signature, 1> output_signatures;
	const std::array<operand_signature, 0> input_signatures;
	mock_command_queue queue;

	return manager.build(
		op,
		make_span(output_signatures),
		make_span(input_signatures),
		queue
	);
}

} // namespace

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

TEST_CASE(
	"program_builder_registration appends its builder factory to a registry",
	"[program_builder_registry]"
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
