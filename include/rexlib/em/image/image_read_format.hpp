// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/backend_priority.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_reader.hpp>

#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_open_options;
class image_probe;

/**
 * @brief The ability of one file format to be read.
 *
 * A format contributes one of these to be able to open files for reading,
 * and one @ref image_write_format to be able to create them. A format that
 * only reads registers only this one and never has to say that it can not
 * write.
 *
 * Formats are collected by an @ref image_read_format_manager, which shows
 * every one of them the same @ref image_probe and opens the file with
 * whichever reports the highest @ref backend_priority.
 */
class REXLIB_API image_read_format
{
public:
	image_read_format() noexcept;
	image_read_format(const image_read_format &other) = delete;
	image_read_format(image_read_format &&other) = delete;
	virtual ~image_read_format();

	image_read_format& operator=(const image_read_format &other) = delete;
	image_read_format& operator=(image_read_format &&other) = delete;

	/**
	 * @brief Get the name of this format.
	 *
	 * Identifies the format in diagnostics and lets a caller tell which one
	 * claimed a file.
	 *
	 * @return std::string The name.
	 */
	virtual std::string get_name() const = 0;

	/**
	 * @brief Report how well this format fits a file.
	 *
	 * Decide from @p probe alone and do not open the file: the probe already
	 * carries its path, its lower case extension and its leading bytes, read
	 * once for every format consulted.
	 *
	 * Return @ref backend_priority::unsupported for a file this format does
	 * not recognize, and something higher than
	 * @ref backend_priority::normal only to displace another format that
	 * recognizes the same file.
	 *
	 * @param probe The file under consideration.
	 * @return backend_priority How well this format fits @p probe.
	 */
	virtual backend_priority
	get_suitability(const image_probe &probe) const = 0;

	/**
	 * @brief Open a file for reading.
	 *
	 * Only called when @ref get_suitability reported something other than
	 * @ref backend_priority::unsupported for @p probe.
	 *
	 * @param probe The file to open.
	 * @param options Which image of the file to open and how it will be
	 * walked.
	 * @return std::unique_ptr<image_reader> The opened reader, never null.
	 * @throws image_format_error If the file is malformed or truncated.
	 * @throws std::out_of_range If the file holds no file at the
	 * requested index.
	 */
	virtual std::unique_ptr<image_reader> open(
		const image_probe &probe,
		const image_open_options &options
	) const = 0;
};

} // namespace em
} // namespace rexlib
