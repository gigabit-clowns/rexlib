// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/backend_priority.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_writer.hpp>

#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_descriptor;
class image_metadata;
class image_probe;

/**
 * @brief One file format that can be written.
 *
 * Judges from an @ref image_probe how well it fits a file, and creates a
 * file it fits, returning an @ref image_writer for it.
 *
 * @see image_read_format
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
	 * @return std::string The name, which identifies the format.
	 */
	virtual std::string get_name() const = 0;

	/**
	 * @brief Report how well this format fits a file.
	 *
	 * The file named by @p probe need not exist, in which case the probe
	 * carries no leading bytes and the decision rests on the extension
	 * alone. Check @ref image_probe::exists rather than assuming there are
	 * leading bytes to read.
	 *
	 * @param probe The file under consideration.
	 * @return backend_priority How well this format fits @p probe.
	 */
	virtual backend_priority
	get_suitability(const image_probe &probe) const = 0;

	/**
	 * @brief Create a file and open it for writing.
	 *
	 * The descriptor is complete, so the file may be laid out in full before
	 * anything is written. Any file already at that path is replaced.
	 *
	 * How the file lays its samples out is the format's own decision, so the
	 * shape comes as an @ref image_descriptor, with no strides or offset.
	 *
	 * @param probe The file to create.
	 * @param descriptor What the file holds. Its data type is what the file
	 * stores rather than what a write will supply, since a write converts.
	 * @param metadata How its samples map onto physical space. A format
	 * writes what of it it can carry and ignores the rest.
	 * @return std::shared_ptr<image_writer> The opened writer, never null.
	 * @throws unsupported_operation_error If this format can not represent
	 * the requested file, such as a rank or a data type it has no encoding
	 * for.
	 * @throws image_file_error If the file could not be created.
	 */
	virtual std::shared_ptr<image_writer> open(
		const image_probe &probe,
		const image_descriptor &descriptor,
		const image_metadata &metadata
	) const = 0;
};

} // namespace em
} // namespace rexlib
