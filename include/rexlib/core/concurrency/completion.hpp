// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{

/**
 * @brief A query-only view of whether one or more units of work have
 * finished.
 *
 * Carries no way to mark itself done: that belongs to the disjoint
 * @ref completion_notifier, which a caller holding this is never handed.
 */
class REXLIB_API completion
{
public:
	completion() = default;
	completion(const completion &other) = default;
	completion(completion &&other) = default;
	virtual ~completion() = default;

	completion& operator=(const completion &other) = default;
	completion& operator=(completion &&other) = default;

	/**
	 * @brief Block the calling thread until this is ready.
	 */
	virtual void wait() const = 0;

	/**
	 * @brief Whether the work behind this has finished.
	 *
	 * @return bool True once every unit of work this stands for has run,
	 * successfully or not.
	 */
	virtual bool is_ready() const noexcept = 0;

	/**
	 * @brief Wait, then rethrow the first exception recorded, if any.
	 *
	 * @throws Whatever the first unit of work to fail threw.
	 */
	virtual void get() const = 0;
};

} // namespace rexlib
