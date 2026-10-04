// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>

namespace rexlib
{

class array;
class completion;

namespace em
{

class image_transaction_plan;
class image_transfer_sanitizer;

/**
 * @brief Loads the regions of a transaction plan into one array.
 *
 * Every region a plan names is read out of its file and into the array. The
 * reads may still be under way when @ref load returns; the completion it
 * returns says when they are done and reports what failed.
 *
 * @par Thread safety
 * @ref load may be called concurrently.
 *
 * @see image_saver
 */
class REXLIB_API image_loader
{
public:
	image_loader() noexcept;
	image_loader(const image_loader &other) = delete;
	image_loader(image_loader &&other) = delete;
	virtual ~image_loader();

	image_loader& operator=(const image_loader &other) = delete;
	image_loader& operator=(image_loader &&other) = delete;

	/**
	 * @brief Load every region a transaction plan names.
	 *
	 * May return before the reads are done. The completion returned is ready
	 * once every region has been read or has failed, and rethrows what the
	 * first failure threw.
	 *
	 * The regions of each file are shown to @p sanitizer beside the extents
	 * of that file and of @p destination, and what it answers is read in
	 * their place. What it refuses is reported through the completion. The
	 * elements of @p destination no region reached are left as they were.
	 *
	 * @param destination Where the regions land.
	 * @param plan The transaction to load.
	 * @param sanitizer What becomes of the regions that do not fit.
	 * @return std::shared_ptr<completion> The completion, never null.
	 * @throws std::invalid_argument If @p sanitizer is null.
	 */
	virtual std::shared_ptr<completion> load(
		array destination,
		const image_transaction_plan &plan,
		std::shared_ptr<const image_transfer_sanitizer> sanitizer
	) const = 0;
};

} // namespace em
} // namespace rexlib
