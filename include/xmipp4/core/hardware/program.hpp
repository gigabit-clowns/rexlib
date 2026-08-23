// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "program_scratch_requirement.hpp"

#include <xmipp4/core/platform/dynamic_shared_object.h>
#include <xmipp4/core/span.hpp>

#include <memory>
#include <cstddef>

namespace rex
{

/**
 * @brief Abstract representation of an executable program.
 *
 * A @c program is a passive and backend-specific descriptor of work to
 * be executed on a device. Execution is driven by @ref command_queue::submit,
 * which records the program on a queue together with its operands.
 *
 * Each concrete program instance is bound to the device it was created for
 * and may only be submitted to queues belonging to that device.
 */
class REXLIB_API program
{
public:
	program() noexcept;
	program(const program &other) = delete;
	program(program &&other) = delete;
	virtual ~program();

	program& operator=(const program &other) = delete;
	program& operator=(program &&other) = delete;

	/**
	 * @brief Queries the scratch buffer requirements for this program.
	 *
	 * The program may require temporary (scratch) memory buffers for execution. 
	 * This method returns a view over the scratch buffer requirements for this 
	 * program, allowing the caller to allocate the necessary buffers before 
	 * program submission. The view remains valid and stable during the lifetime
	 * of the object.
	 * 
	 * If the returned list is empty, the program requires no scratch buffers 
	 * and scratch does not need to be bound in execution. Otherwise, scratch 
	 * binding in @ref command must contain one buffer for each requirement in 
	 * the order returned. Each buffer must have a size >= the corresponding
	 * requirement's size and alignment >= the requirement's alignment. Contents 
	 * are undefined on entry and on completion, so callers must not store 
	 * persistent data in them and may reuse the same buffers for multiple 
	 * programs as long as their executions do not overlap. 
	 *
	 * Returns an empty span by default (no scratch required).
	 *
	 * @return A non-owning view of the scratch buffer requirements.
	 *
	 * @see program_scratch_requirement for details on individual requirement
	 * specifications.
	 */
	virtual
	span<const program_scratch_requirement> get_scratch_requirements() const;
};

} // namespace rex
