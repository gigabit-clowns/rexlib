// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/concurrency/completion_notifier.hpp>
#include <rexlib/core/concurrency/executor.hpp>
#include <rexlib/core/concurrency/task.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>

namespace rexlib
{

/**
 * @brief An executor that runs a task on the calling thread, before
 * submit returns.
 *
 * Spawns no thread and keeps no state. A completion fed by a notifier
 * submitted here is therefore already resolved by the time submit is
 * back.
 */
class synchronous_executor final
	: public executor
{
public:
	REXLIB_API
	synchronous_executor() noexcept;

	synchronous_executor(const synchronous_executor &other) = delete;
	synchronous_executor(synchronous_executor &&other) = delete;

	REXLIB_API
	~synchronous_executor() override;

	synchronous_executor&
	operator=(const synchronous_executor &other) = delete;
	synchronous_executor&
	operator=(synchronous_executor &&other) = delete;

	/**
	 * @brief Run a unit of work immediately, on the calling thread.
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
};

} // namespace rexlib
