// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/backend_priority.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_writer.hpp>

#include <cstddef>
#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_metadata;
class image_open_options;
class image_probe;

/**
 * @brief The ability of one file format to be written.
 *
 * The counterpart of @ref image_read_format, registered separately so that a
 * format may offer one without the other and so that a program that only
 * reads never reaches the writing side at all.
 */
class REXLIB_API image_write_format
{
public:
	image_write_format() noexcept;
	image_write_format(const image_write_format &other) = delete;
	image_write_format(image_write_format &&other) = delete;
	virtual ~image_write_format();

	image_write_format& operator=(const image_write_format &other) = delete;
	image_write_format& operator=(image_write_format &&other) = delete;

	/**
	 * @brief Get the name of this format.
	 *
	 * @return std::string The name.
	 */
	virtual std::string get_name() const = 0;

	/**
	 * @brief Report how well this format fits a file.
	 *
	 * The file named by @p probe usually does not exist yet, in which case
	 * the probe carries no leading bytes and the decision rests on the
	 * extension alone. Check @ref image_probe::exists rather than assuming
	 * there are leading bytes to read.
	 *
	 * @param probe The file under consideration.
	 * @return backend_priority How well this format fits @p probe.
	 */
	virtual backend_priority
	get_suitability(const image_probe &probe) const = 0;

	/**
	 * @brief Create a file and open it for writing.
	 *
	 * The extents are complete, so the file may be laid out in full before
	 * anything is written. Any file already at that path is replaced.
	 *
	 * The shape is given as extents and a data type rather than as an
	 * @ref array_descriptor because how the file lays its elements out is
	 * the format's own decision: strides and an offset would be stated by
	 * the caller and then ignored.
	 *
	 * @param probe The file to create.
	 * @param options How the file will be walked.
	 * @param extents Extents of the file to create, slowest axis first.
	 * @param data_type Data type of its elements. A format converts to
	 * whatever it encodes, so this is what the file holds rather than what
	 * a write will supply.
	 * @param metadata How its samples map onto physical space. A format
	 * writes what of it it can carry and ignores the rest.
	 * @return std::unique_ptr<image_writer> The opened writer, never null.
	 * @throws invalid_operation_error If this format can not represent the
	 * requested file, such as a rank or a data type it has no encoding for.
	 * @throws image_format_error If the file could not be created.
	 */
	virtual std::unique_ptr<image_writer> open(
		const image_probe &probe,
		const image_open_options &options,
		span<const std::size_t> extents,
		numerical_type data_type,
		const image_metadata &metadata
	) const = 0;
};

} // namespace em
} // namespace rexlib
