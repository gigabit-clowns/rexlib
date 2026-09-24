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

/**
 * @brief Writes the regions of a transaction plan out of one array.
 *
 * Every region a plan names is written out of the array and into its file.
 * The writes may still be under way when @ref write returns; the completion
 * it returns says when they are done and reports what failed.
 *
 * @par Thread safety
 * @ref write may be called concurrently.
 *
 * @see image_source
 */
class REXLIB_API image_sink
{
public:
	image_sink() noexcept;
	image_sink(const image_sink &other) = delete;
	image_sink(image_sink &&other) = delete;
	virtual ~image_sink();

	image_sink& operator=(const image_sink &other) = delete;
	image_sink& operator=(image_sink &&other) = delete;

	/**
	 * @brief Write every region a transaction plan names.
	 *
	 * Returns before the writes are done. The completion returned is ready
	 * once every region has been written or has failed, and rethrows what
	 * the first failure threw.
	 *
	 * Every region must fit the file it names and @p source both. One that
	 * does not is reported through the completion rather than shortened to
	 * fit.
	 *
	 * @param source The values to write.
	 * @param plan The transaction to write.
	 * @return std::shared_ptr<completion> The completion, never null.
	 */
	virtual std::shared_ptr<completion> write(
		const_array source,
		const image_transaction_plan &plan
	) const = 0;

	/**
	 * @brief Make everything written through this sink reach the storage.
	 *
	 * Only the writes whose completions are ready are sure to be included.
	 *
	 * @throws image_format_error If the pending writes could not be
	 * completed.
	 */
	virtual void flush() = 0;
};

} // namespace em
} // namespace rexlib
