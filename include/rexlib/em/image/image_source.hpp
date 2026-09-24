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
	 * A region transfers the intersection of what its file holds at its file
	 * offset with what @p destination holds at its array offset, up to the
	 * extents of the plan. The intersection begins where the region begins
	 * on either side, so a region reaching past either of them is shortened
	 * rather than refused, and one that reaches past both is shortened by
	 * whichever runs out first. A region reaching past a side along an axis
	 * the extents of the plan do not cover, which spans a single position,
	 * transfers nothing at all. The elements of @p destination no region
	 * reached are left as they were.
	 *
	 * @param destination Where the regions land.
	 * @param plan The transaction to read.
	 * @return std::shared_ptr<completion> The completion, never null.
	 */
	virtual std::shared_ptr<completion> read(
		array destination,
		const image_transaction_plan &plan
	) const = 0;
};

} // namespace em
} // namespace rexlib
