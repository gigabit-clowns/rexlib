// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{

/**
 * @brief One unit of work an @ref executor can run.
 *
 * @ref run is expected not to let an exception escape when submitted
 * alongside a @ref completion_notifier: whichever @ref executor runs it
 * catches what it throws and reports it through that notifier instead.
 */
class REXLIB_API task
{
public:
	task() = default;
	task(const task &other) = default;
	task(task &&other) = default;
	virtual ~task() = default;

	task& operator=(const task &other) = default;
	task& operator=(task &&other) = default;

	/**
	 * @brief Run this unit of work.
	 */
	virtual void run() = 0;
};

} // namespace rexlib
