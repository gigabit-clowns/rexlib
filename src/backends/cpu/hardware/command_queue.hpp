// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/hardware/command_queue.hpp>

#include <memory>

namespace rex
{
namespace cpu
{

class thread_pool;

/**
 * @brief CPU implementation of @ref command_queue.
 */
class command_queue final
	: public rex::command_queue
{
public:
	/**
	 * @brief Construct a queue over a set of threads.
	 *
	 * The pool is named rather than reached for, so that the only thing in
	 * this backend that owns threads is the @ref device a queue was made by.
	 * Nothing static holds one, which is what keeps the workers being joined
	 * while the program is still running rather than while it is exiting.
	 *
	 * @param pool The threads the programs submitted here may spread
	 * themselves over.
	 *
	 * @throws std::invalid_argument If @p pool is null.
	 */
	explicit command_queue(std::shared_ptr<thread_pool> pool);
	~command_queue() override = default;

	void submit(const command &cmd) override;
	void signal(event &event) override;
	void wait(const event &event) override;

	/**
	 * @brief Get the threads this queue runs its programs over.
	 *
	 * @return thread_pool& The pool.
	 */
	thread_pool& get_thread_pool() const noexcept;

	static
	command_queue* try_cast(rex::command_queue &queue) noexcept;
	static
	const command_queue* try_cast(const rex::command_queue &queue) noexcept;

private:
	std::shared_ptr<thread_pool> m_pool;
};

} // namespace cpu
} // namespace rex
