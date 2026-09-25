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

class image_probe;

/**
 * @brief One file format that can be read.
 *
 * Judges from an @ref image_probe how well it fits a file, and opens a file
 * it fits as an @ref image_reader.
 *
 * @see image_write_format
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
	 * @return std::string The name, which identifies the format.
	 */
	virtual std::string get_name() const = 0;

	/**
	 * @brief Report how well this format fits a file.
	 *
	 * Decide from @p probe alone, without opening the file: the probe
	 * carries its path, its lower case extension and its leading bytes.
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
	 * @pre @ref get_suitability does not report @p probe as
	 * @ref backend_priority::unsupported.
	 *
	 * @param probe The file to open.
	 * @return std::shared_ptr<image_reader> The opened reader, never null.
	 * @throws image_file_error If the file can not be reached.
	 * @throws image_format_error If the file is malformed or truncated.
	 */
	virtual std::shared_ptr<image_reader> open(
		const image_probe &probe
	) const = 0;
};

} // namespace em
} // namespace rexlib
