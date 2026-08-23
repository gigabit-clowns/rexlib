// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/program.hpp>
#include <xmipp4/core/span.hpp>

namespace rex
{

class buffer;

namespace cpu
{

class thread_pool;

/**
 * @brief CPU base class for executable programs.
 */
class REXLIB_API program
	: public xmipp4::program
{
public:
	/**
	 * @brief Immediately execute the the program.
	 *
	 * The pool travels with the call rather than with the program because a
	 * program is built once and run many times, possibly against queues that
	 * do not share their threads. It is the queue running the program that
	 * knows which threads the program may use, so it says so here.
	 *
	 * A program is free to ignore it and run on the calling thread. Whatever
	 * it does, the call returns only once the program has finished: the
	 * operands are spans the caller owns, and they stop being valid when
	 * `command_queue::submit` returns.
	 *
	 * @param outputs Output buffers. Neither may be null.
	 * @param inputs Input buffers. Neither may be null.
	 * @param scratch Workspace buffers. Neither may be null.
	 * @param pool The threads the program may spread itself over.
	 */
	virtual void execute(
		span<const std::shared_ptr<buffer>> outputs,
		span<const std::shared_ptr<const buffer>> inputs,
		span<const std::shared_ptr<buffer>> scratch,
		thread_pool &pool
	) const = 0;
};

} // namespace cpu
} // namespace rex
