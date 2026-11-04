// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/program.hpp>
#include <rexlib/core/span.hpp>

#include <memory>

namespace sample
{

/**
 * @brief A program of the sample backend.
 */
class sample_program
	: public rexlib::program
{
public:
	/**
	 * @brief Run the program on the calling thread.
	 *
	 * @param outputs Output buffers.
	 * @param inputs Input buffers.
	 * @param scratch Workspace buffers, one per scratch requirement.
	 */
	virtual void execute(
		rexlib::span<const std::shared_ptr<rexlib::buffer>> outputs,
		rexlib::span<const std::shared_ptr<const rexlib::buffer>> inputs,
		rexlib::span<const std::shared_ptr<rexlib::buffer>> scratch
	) const = 0;
};

} // namespace sample
