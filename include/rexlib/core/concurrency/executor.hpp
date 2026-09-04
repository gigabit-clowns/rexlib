// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/concurrency/completion_notifier.hpp>
#include <rexlib/core/concurrency/task.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>

namespace rexlib
{

/**
 * @brief Where a @ref task is handed off to run, now or later.
 *
 * Runs the submitted task and reports its outcome to the notifier it was
 * submitted with exactly once, catching whatever the task throws rather
 * than letting it escape. A task carries no notion of a completion
 * itself; that pairing is made here, by whoever calls submit.
 *
 * @par Thread safety
 * submit may be called concurrently.
 */
class REXLIB_API executor
{
public:
	executor() = default;
	executor(const executor &other) = default;
	executor(executor &&other) = default;
	virtual ~executor() = default;

	executor& operator=(const executor &other) = default;
	executor& operator=(executor &&other) = default;

	/**
	 * @brief Hand a unit of work off to this executor.
	 *
	 * @param t The work to run. Must not be null.
	 * @param notifier Where the outcome is reported. Must not be null.
	 */
	virtual void submit(
		std::unique_ptr<task> t,
		std::shared_ptr<completion_notifier> notifier
	) = 0;
};

} // namespace rexlib
