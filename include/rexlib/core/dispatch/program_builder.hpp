// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operation_id.hpp"

#include "../platform/dynamic_shared_object.h"
#include "../span.hpp"
#include "../numerical/numerical_type.hpp"
#include "../backend_priority.hpp"

#include <memory>

namespace rexlib
{

class program;
class command_queue;


class operation;
class operand_signature;
class program_cache;

/**
 * @brief Abstract representation of a factory class for operation programs
 */
class REXLIB_API program_builder
{
public:
	program_builder() noexcept;
	program_builder(const program_builder &other) = delete;
	program_builder(program_builder &&other) = delete;
	virtual ~program_builder();

	program_builder&
	operator=(const program_builder &other) = delete;
	program_builder&
	operator=(program_builder &&other) = delete;

	/**
	 * @brief Get the operation identifier for which this builder is
	 * specialized.
	 *
	 * @return operation_id The operation ID.
	 */
	virtual operation_id get_operation_id() const noexcept = 0;

	/**
	 * @brief Get the suitability of this builder for a given launch
	 * configuration.
	 *
	 * @param operation The operation. Its ID must be equal to the ID returned
	 * by get_operation_id.
	 * @param output_signatures The output array signatures involved in the
	 * operation.
	 * @param input_signatures The input array signatures involved in the
	 * operation.
	 * @param queue The command queue where the built program is
	 * intended to be used.
	 * @return backend_priority The suitability of this builder for the
	 * requested launch configuration.
	 */
	virtual backend_priority get_suitability(
		const operation &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		command_queue &queue
	) const = 0;

	/**
	 * @brief Build an executable operation program for a given launch
	 * configuration.
	 *
	 * This method may not be called if get_suitability() returns
	 * backend_priority::unsupported.
	 *
	 * @param operation The operation. Its ID must be equal to the ID returned
	 * by get_operation_id.
	 * @param output_signatures The output array signatures involved in the
	 * operation.
	 * @param input_signatures The input array signatures involved in the
	 * operation.
	 * @param queue The command queue where the built program is
	 * intended to be used.
	 * @param cache Optional cache for backend-private resources. When not
	 * null, the builder may consult and update it to reuse expensive
	 * resources (FFT plans, compiled kernels, workspaces, ...) across
	 * builds. Implementations are free to ignore it.
	 * @return std::shared_ptr<program> The executable program suited
	 * for the requested operation and signature.
	 */
	virtual
	std::shared_ptr<program> build(
		const operation &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		command_queue &queue,
		program_cache *cache
	) const = 0;
};

} // namespace rexlib
