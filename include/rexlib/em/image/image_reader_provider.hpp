// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_descriptor.hpp>

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
 * Whether a reader is opened afresh, kept or shared is up to the provider,
 * and so is how a path becomes a reader at all: through formats, from
 * readers already open, or from anywhere else. Whoever asks names a path and
 * gets something it can read, and learns neither.
 *
 * @par Thread safety
 * A provider may be asked for readers concurrently.
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
	 * Shared ownership rather than a reference, so that a reader the
	 * provider stops keeping stays alive for as long as it is still read
	 * through.
	 *
	 * @param path Path to the file to read.
	 * @return std::shared_ptr<const image_reader> The reader, never null.
	 * @throws image_file_error If the file does not exist or can not be read.
	 * @throws unsupported_operation_error If no format can read the file.
	 * @throws image_format_error If the file is malformed or truncated.
	 */
	virtual std::shared_ptr<const image_reader>
	acquire(const std::string &path) = 0;
};

/**
 * @brief Get the descriptor of a file through a provider.
 *
 * A copy the caller owns, so the shape of a file is answered without the
 * reader being kept. Whether the file is opened to answer depends on
 * @p readers, which may already hold a reader over it.
 *
 * @param readers Where the file becomes a reader.
 * @param path Path to the file.
 * @return image_descriptor The descriptor of the file.
 * @throws image_file_error If the file does not exist or can not be read.
 * @throws unsupported_operation_error If no format can read the file.
 * @throws image_format_error If the file is malformed or truncated.
 */
REXLIB_API
image_descriptor query_descriptor(
	image_reader_provider &readers,
	const std::string &path
);

} // namespace em
} // namespace rexlib
