// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>

namespace rexlib
{

class completion;
class const_array;

namespace em
{

class image_transaction_plan;
class image_transfer_sanitizer;

/**
 * @brief Saves the regions of a transaction plan out of one array.
 *
 * Every region a plan names is written out of the array and into its file.
 * The writes may still be under way when @ref save returns; the completion
 * it returns says when they are done and reports what failed.
 *
 * @par Thread safety
 * @ref save may be called concurrently.
 *
 * @see image_loader
 */
class REXLIB_API image_saver
{
public:
	image_saver() noexcept;
	image_saver(const image_saver &other) = delete;
	image_saver(image_saver &&other) = delete;
	virtual ~image_saver();

	image_saver& operator=(const image_saver &other) = delete;
	image_saver& operator=(image_saver &&other) = delete;

	/**
	 * @brief Save every region a transaction plan names.
	 *
	 * Returns before the writes are done. The completion returned is ready
	 * once every region has been written or has failed, and rethrows what
	 * the first failure threw.
	 *
	 * The regions of each file are shown to @p sanitizer beside the extents
	 * of that file and of @p source, and what it answers is written in
	 * their place. What it refuses is reported through the completion.
	 *
	 * @param source The values to save.
	 * @param plan The transaction to save.
	 * @param sanitizer What becomes of the regions that do not fit.
	 * @return std::shared_ptr<completion> The completion, never null.
	 * @throws std::invalid_argument If @p sanitizer is null.
	 */
	virtual std::shared_ptr<completion> save(
		const_array source,
		const image_transaction_plan &plan,
		std::shared_ptr<const image_transfer_sanitizer> sanitizer
	) const = 0;
};

} // namespace em
} // namespace rexlib
