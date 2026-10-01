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
 * @brief Reads the regions of a transaction plan into one array.
 *
 * Every region a plan names is read out of its file and into the array. The
 * reads may still be under way when @ref read returns; the completion it
 * returns says when they are done and reports what failed.
 *
 * @par Thread safety
 * @ref read may be called concurrently.
 *
 * @see image_sink
 */
class REXLIB_API image_source
{
public:
	image_source() noexcept;
	image_source(const image_source &other) = delete;
	image_source(image_source &&other) = delete;
	virtual ~image_source();

	image_source& operator=(const image_source &other) = delete;
	image_source& operator=(image_source &&other) = delete;

	/**
	 * @brief Read every region a transaction plan names.
	 *
	 * Returns before the reads are done. The completion returned is ready
	 * once every region has been read or has failed, and rethrows what the
	 * first failure threw.
	 *
	 * The regions of each file are shown to @p sanitizer beside the extents
	 * of that file and of @p destination, and what it answers is read in
	 * their place. What it refuses is reported through the completion. The
	 * elements of @p destination no region reached are left as they were.
	 *
	 * @param destination Where the regions land.
	 * @param plan The transaction to read.
	 * @param sanitizer What becomes of the regions that do not fit.
	 * @return std::shared_ptr<completion> The completion, never null.
	 * @throws std::invalid_argument If @p sanitizer is null.
	 */
	virtual std::shared_ptr<completion> read(
		array destination,
		const image_transaction_plan &plan,
		std::shared_ptr<const image_transfer_sanitizer> sanitizer
	) const = 0;
};

} // namespace em
} // namespace rexlib
