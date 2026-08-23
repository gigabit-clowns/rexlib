// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <memory>

namespace xmipp4
{
namespace cpu
{

class thread_pool_implementation;

/**
 * @brief A fixed set of threads a loop's iterations are spread over.
 *
 * The pool is fork-join and nothing else: @ref run publishes one job, every
 * participant claims chunks of it until there are none left, and the call
 * returns only once every chunk has finished. There is no queue, no future
 * and no way for work to outlive the call, which is what lets a program keep
 * being handed raw pointers into buffers whose lifetime ends when
 * `command_queue::submit` returns. See @ref xmipp4::command.
 *
 * The calling thread participates. A pool of no workers therefore costs
 * exactly one indirect call over running the loop inline, which is what makes
 * it safe to route every loop of the backend through it rather than only the
 * ones expected to be large.
 *
 * Work below the grain size is not split at all. The grain is stated by the
 * caller because only the caller knows what one iteration costs: an
 * elementwise iteration is one arithmetic operation, a linalg one is a whole
 * matrix product.
 */
class thread_pool
{
public:
	/**
	 * @brief The type-erased body of a parallel loop.
	 *
	 * @param context Opaque pointer, handed back untouched.
	 * @param begin The first iteration this call covers.
	 * @param end The past-the-end iteration this call covers.
	 */
	using body_function =
		void (*)(const void *context, std::size_t begin, std::size_t end);

	/**
	 * @brief Construct a pool with a given number of worker threads.
	 *
	 * @param worker_count Threads to spawn, not counting the caller. Zero
	 * makes every @ref run serial, which is the shape a test asks for when it
	 * wants the parallel path exercised on a single deterministic thread.
	 */
	REXLIB_API
	explicit thread_pool(std::size_t worker_count);

	thread_pool(const thread_pool &other) = delete;
	thread_pool(thread_pool &&other) = delete;

	REXLIB_API
	~thread_pool();

	thread_pool& operator=(const thread_pool &other) = delete;
	thread_pool& operator=(thread_pool &&other) = delete;

	/**
	 * @brief Get the number of threads a job is spread over.
	 *
	 * The workers plus the calling thread, so it is never zero.
	 *
	 * @return std::size_t The number of participants.
	 */
	REXLIB_API
	std::size_t get_size() const noexcept;

	/**
	 * @brief Run a loop, splitting it across this pool.
	 *
	 * The iteration space `[0, count)` is cut into as many contiguous chunks
	 * as there are participants, never into chunks shorter than
	 * @p grain_size, and @p body is invoked once per chunk. The partition
	 * depends only on @p count, @p grain_size and @ref get_size, never on
	 * timing; which thread runs which chunk does depend on timing.
	 *
	 * Runs the whole range inline, on the calling thread, when the pool has
	 * no worker, when the work is below one grain, and when a job is already
	 * running on this pool. That last case covers both a second host thread
	 * submitting at the same time and a body reaching this again: a job holds
	 * the pool for the whole of its duration, so a nested call finds it taken
	 * and runs itself rather than waiting for threads that are the very ones
	 * waiting on it. Nesting is therefore safe without a caller having to
	 * know it is nested.
	 *
	 * @param count Number of iterations.
	 * @param grain_size Smallest number of iterations worth handing to a
	 * thread of its own. Treated as one when zero.
	 * @param body The loop body. Invoked concurrently on one and the same
	 * object, so any state a chunk needs to itself must be created inside it.
	 * @param context Passed to @p body untouched.
	 *
	 * @throws Whatever @p body throws. Every chunk is guarded, the exception
	 * raised by the lowest numbered chunk is kept, the chunks not yet claimed
	 * are abandoned, and the exception is rethrown here, on the calling
	 * thread, once every chunk already running has finished. The pool is
	 * usable again afterwards.
	 */
	REXLIB_API
	void run(
		std::size_t count,
		std::size_t grain_size,
		body_function body,
		const void *context
	);

	/**
	 * @brief How many workers a pool spawns when nothing else says otherwise.
	 *
	 * One participant per hardware thread ends up running a loop, and the
	 * thread that asked for it is one of them, so this is one fewer than that.
	 *
	 * The number may be capped from the environment, through
	 * REX_NUM_THREADS. The machine is a run time property, so a compile
	 * time knob could not answer this: a user under a batch scheduler or
	 * inside a process pool needs to cap the library from outside it, which
	 * is what every peer library offers a variable for. A value of one means
	 * no worker at all, and so a fully serial library.
	 *
	 * @return std::size_t The worker count, which may be zero.
	 */
	REXLIB_API
	static std::size_t get_default_worker_count();

private:
	REX_STD_MEMBER_INTERFACE
	std::unique_ptr<thread_pool_implementation> m_implementation;
};

} // namespace cpu
} // namespace xmipp4
