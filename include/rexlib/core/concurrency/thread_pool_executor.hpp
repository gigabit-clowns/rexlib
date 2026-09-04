// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/concurrency/completion_notifier.hpp>
#include <rexlib/core/concurrency/executor.hpp>
#include <rexlib/core/concurrency/task.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <memory>

namespace rexlib
{

/**
 * @brief An executor backed by a fixed pool of worker threads.
 *
 * Every worker pulls the next submitted task from one shared queue,
 * runs it, and reports its outcome to the notifier it was submitted
 * with, catching whatever the task threw. Submitting never blocks the
 * caller and never runs the task itself; a task queued while every
 * worker is busy simply waits its turn. The queue is unbounded.
 *
 * The worker count should track the concurrency the destination the
 * tasks read from or write to actually supports, not the number of CPU
 * cores: a worker mostly blocks in I/O rather than computing, which is
 * why this is unrelated to @ref cpu::thread_pool.
 *
 * The destructor waits for every already-queued task to run before
 * returning, submitted or not yet started.
 *
 * @par Thread safety
 * submit may be called concurrently.
 */
class thread_pool_executor final
	: public executor
{
public:
	/**
	 * @brief Construct a pool with a given number of worker threads.
	 *
	 * @param worker_count Threads to spawn. Must be greater than zero.
	 * @throws std::invalid_argument If @p worker_count is zero.
	 */
	REXLIB_API
	explicit thread_pool_executor(std::size_t worker_count);

	thread_pool_executor(const thread_pool_executor &other) = delete;
	thread_pool_executor(thread_pool_executor &&other) = delete;

	REXLIB_API
	~thread_pool_executor() override;

	thread_pool_executor&
	operator=(const thread_pool_executor &other) = delete;
	thread_pool_executor&
	operator=(thread_pool_executor &&other) = delete;

	/**
	 * @brief Get the number of worker threads.
	 *
	 * @return std::size_t The worker count passed at construction.
	 */
	REXLIB_API
	std::size_t get_worker_count() const noexcept;

	/**
	 * @brief Queue a unit of work for a worker to run.
	 *
	 * @param t The work to run. Must not be null.
	 * @param notifier Where the outcome is reported. Must not be null.
	 * @throws std::invalid_argument If @p t or @p notifier is null.
	 */
	REXLIB_API
	void submit(
		std::unique_ptr<task> t,
		std::shared_ptr<completion_notifier> notifier
	) override;

private:
	class implementation;
	REXLIB_STD_MEMBER_INTERFACE
	std::unique_ptr<implementation> m_implementation;
};

} // namespace rexlib
