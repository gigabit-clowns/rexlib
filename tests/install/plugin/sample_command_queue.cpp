// SPDX-License-Identifier: GPL-3.0-only

#include "sample_command_queue.hpp"

#include "sample_program.hpp"

#include <rexlib/core/hardware/command.hpp>
#include <rexlib/core/hardware/command_token.hpp>

#include <stdexcept>

namespace sample
{

rexlib::command_token sample_command_queue::submit(rexlib::command cmd)
{
	const auto *program =
		dynamic_cast<const sample_program*>(cmd.get_program().get());
	if (program == nullptr)
	{
		throw std::invalid_argument(
			"The sample queue runs the programs of the sample backend only."
		);
	}

	for (const auto &dependency : cmd.get_dependencies())
	{
		dependency.wait();
	}

	program->execute(cmd.get_outputs(), cmd.get_inputs(), cmd.get_scratch());

	return rexlib::command_token();
}

} // namespace sample
