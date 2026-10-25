// SPDX-License-Identifier: GPL-3.0-only

#include "cpu_command_queue.hpp"

#include <rexlib/backends/cpu/cpu_program.hpp>
#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/core/hardware/command.hpp>

#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace cpu
{

cpu_command_queue::cpu_command_queue(std::shared_ptr<thread_pool> pool)
	: m_pool(std::move(pool))
{
	if (!m_pool)
	{
		throw std::invalid_argument(
			"cpu_command_queue::cpu_command_queue: A thread pool is required."
		);
	}
}

command_token cpu_command_queue::submit(command cmd)
{
	const auto& prog = cmd.get_program();
	if (!prog)
	{
		throw std::invalid_argument(
			"cpu_command_queue::submit: Provided command does not have an "
			"associated program."
		);
	}

	const auto& cpu_prog = dynamic_cast<const cpu_program&>(*prog);

	for (const auto &dependency : cmd.get_dependencies())
	{
		dependency.wait();
	}

	// Synchronous: threading a program does not change that, the pool being
	// fork-join.
	cpu_prog.execute(
		cmd.get_outputs(),
		cmd.get_inputs(),
		cmd.get_scratch(),
		*m_pool
	);

	return command_token();
}

thread_pool& cpu_command_queue::get_thread_pool() const noexcept
{
	return *m_pool;
}

cpu_command_queue*
cpu_command_queue::try_cast(command_queue &queue) noexcept
{
	return dynamic_cast<cpu_command_queue*>(&queue);
}

const cpu_command_queue*
cpu_command_queue::try_cast(const command_queue &queue) noexcept
{
	return dynamic_cast<const cpu_command_queue*>(&queue);
}

} // namespace cpu
} // namespace rexlib
