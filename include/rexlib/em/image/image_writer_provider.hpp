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
 * @brief Interface to serve @ref image_writer given a path.
 *
 * The @ref image_writer creation mechanism is left to the implementation.
 *
 * @par Thread safety
 * A provider may be asked for writers concurrently.
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
	 * Shared ownership rather than a reference, so that a writer the
	 * provider stops keeping stays alive for as long as it is still written
	 * through.
	 *
	 * @param path Path to the file to write.
	 * @return std::shared_ptr<image_writer> The writer, never null.
	 * @throws std::out_of_range If this provider serves no such file.
	 * @throws unsupported_operation_error If no format can create the file,
	 * or the chosen one can not represent it.
	 * @throws image_file_error If the file could not be created.
	 */
	virtual std::shared_ptr<image_writer>
	acquire(const std::string &path) = 0;
};

} // namespace em
} // namespace rexlib
