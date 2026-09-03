// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_writer;

/**
 * @brief Where a path becomes an open @ref image_writer.
 *
 * The counterpart of @ref image_reader_provider, and as small: a consumer
 * names a file and gets something it can write. How that file came to be
 * writable at all — what extents and data type it was created with, and when
 * it is finished — is lifecycle the owner of the provider drives, so it
 * belongs to the implementation rather than here.
 *
 * @ref flush has no counterpart on the reading side because tier one's
 * @ref image_reader has nothing to flush. It is here rather than on the
 * consumer because the provider owns the writers and is the only thing that
 * knows all of them.
 *
 * A provider may be asked for writers concurrently. It does not follow that
 * the writers may be written concurrently: tier one allows that only for
 * regions that do not overlap, and nothing here changes it.
 */
class REXLIB_API image_writer_provider
{
public:
	image_writer_provider() noexcept;
	image_writer_provider(const image_writer_provider &other) = delete;
	image_writer_provider(image_writer_provider &&other) = delete;
	virtual ~image_writer_provider();

	image_writer_provider&
	operator=(const image_writer_provider &other) = delete;
	image_writer_provider&
	operator=(image_writer_provider &&other) = delete;

	/**
	 * @brief Get a writer over one file.
	 *
	 * Shared ownership rather than a reference, so that a writer an
	 * implementation stops keeping stays alive as long as a transaction is
	 * still writing through it.
	 *
	 * @param path Path to the file to write.
	 * @return std::shared_ptr<image_writer> The writer, never null.
	 * @throws std::out_of_range If this provider serves no such file.
	 * @throws invalid_operation_error If no format can create the file.
	 * @throws image_format_error If the file could not be created.
	 */
	virtual std::shared_ptr<image_writer>
	acquire(const std::string &path) = 0;

	/**
	 * @brief Make everything written through this provider reach the
	 * storage.
	 *
	 * Flushes every writer it holds open and none it does not.
	 *
	 * @throws image_format_error If the pending writes could not be
	 * completed.
	 */
	virtual void flush() = 0;
};

} // namespace em
} // namespace rexlib
