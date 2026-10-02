// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/backend_priority.hpp>
#include <rexlib/core/dispatch/operand_signature.hpp>
#include <rexlib/core/dispatch/operation_id.hpp>
#include <rexlib/core/dispatch/program_builder.hpp>
#include <rexlib/core/dispatch/program_manager.hpp>

#include "../mock/mock_operation.hpp"
#include "../../hardware/mock/mock_command_queue.hpp"
#include "../../hardware/mock/mock_program.hpp"

#include <array>
#include <memory>

namespace rexlib
{
namespace test
{

/**
 * @brief The operation @ref stub_program_builder builds programs for.
 */
struct stub_operation
	: mock_operation
{
	~stub_operation() override = default;
};

/**
 * @brief A builder that builds a mock program for @ref stub_operation.
 *
 * A concrete class rather than a trompeloeil mock: a registry stores
 * factories that create fresh instances on demand, which does not fit
 * per-instance mock expectations. Only get_operation_id and build are
 * exercised through a manager.
 */
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

/**
 * @brief Create a fresh @ref stub_program_builder.
 *
 * @return std::unique_ptr<program_builder> The builder.
 */
inline std::unique_ptr<program_builder> make_stub_builder()
{
	return std::make_unique<stub_program_builder>();
}

/**
 * @brief Ask a manager for a program for @ref stub_operation.
 *
 * @param manager The manager to ask.
 * @return std::shared_ptr<program> The program the manager built.
 */
inline std::shared_ptr<program>
build_stub_operation(const program_manager &manager)
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

} // namespace test
} // namespace rexlib
