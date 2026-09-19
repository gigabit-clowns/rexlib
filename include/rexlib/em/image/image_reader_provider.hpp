// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

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
 * @par Thread safety
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

/**
 * @brief Get the extents of a file through a provider.
 *
 * Asking a reader directly returns extents it owns, which a caller that let
 * go of the reader in the same expression outlives. This owns what it
 * returns, so the shape of a file is answered without the reader being kept
 * or leaked into the caller.
 *
 * The file is opened to answer, which is what @p readers may or may not have
 * to do: one keeping the readers it was asked for answers a repeated question
 * without opening anything.
 *
 * @param readers Where the file becomes a reader.
 * @param path Path to the file.
 * @return std::vector<std::size_t> The extents of the file, slowest axis
 * first.
 * @throws invalid_operation_error If no format recognizes the file.
 * @throws image_format_error If the file is malformed or truncated.
 */
REXLIB_API
std::vector<std::size_t> query_extents(
	image_reader_provider &readers,
	const std::string &path
);

/**
 * @brief Get the extents of one image or volume of a file through a provider.
 *
 * The trailing @ref image_reader::get_core_rank extents of the file, so the
 * axes it stacks along are left out. That is the shape of a single image or
 * volume, which is what the destination of a batch carries beside its leading
 * extent.
 *
 * @param readers Where the file becomes a reader.
 * @param path Path to the file.
 * @return std::vector<std::size_t> The extents of one image or volume.
 * @throws invalid_operation_error If no format recognizes the file.
 * @throws image_format_error If the file is malformed or truncated.
 */
REXLIB_API
std::vector<std::size_t> query_core_extents(
	image_reader_provider &readers,
	const std::string &path
);

} // namespace em
} // namespace rexlib
