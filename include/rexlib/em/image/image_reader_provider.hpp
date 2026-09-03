// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_reader;

/**
 * @brief Where a path becomes an open @ref image_reader.
 *
 * One method, so that whether a handle is opened afresh, kept or shared is a
 * policy of the implementation rather than a fact of life for the code that
 * reads through it. A consumer names a file and gets something it can read;
 * it never learns which.
 *
 * How a path becomes a reader is deliberately not stated here either. The
 * implementations bundled with the library go through an
 * @ref image_read_format_manager, but one serving readers from a plugin, a
 * shared memory segment or a set already open fits the same interface with
 * no format manager anywhere in it.
 *
 * A provider may be asked for readers concurrently, since that is what a
 * transaction in flight does.
 */
class REXLIB_API image_reader_provider
{
public:
	image_reader_provider() noexcept;
	image_reader_provider(const image_reader_provider &other) = delete;
	image_reader_provider(image_reader_provider &&other) = delete;
	virtual ~image_reader_provider();

	image_reader_provider&
	operator=(const image_reader_provider &other) = delete;
	image_reader_provider&
	operator=(image_reader_provider &&other) = delete;

	/**
	 * @brief Get a reader over one file.
	 *
	 * Shared ownership rather than a reference, so that a reader an
	 * implementation stops keeping stays alive as long as a transaction is
	 * still reading through it.
	 *
	 * @param path Path to the file to read.
	 * @return std::shared_ptr<const image_reader> The reader, never null.
	 * @throws invalid_operation_error If no format recognizes the file.
	 * @throws image_format_error If the file is malformed or truncated.
	 */
	virtual std::shared_ptr<const image_reader>
	acquire(const std::string &path) = 0;
};

} // namespace em
} // namespace rexlib
