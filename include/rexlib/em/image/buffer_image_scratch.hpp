// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_scratch.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

namespace rexlib
{

class buffer;

namespace em
{

class image_location;
class image_reader_provider;

/**
 * @brief A scratch that stores its copies in one memory buffer.
 *
 * It holds the images that a list of locations names, as far as they fit in
 * the buffer. Each file gets one entry, and the entries are stored one
 * after another in the buffer. The buffer decides where the copies live:
 * in main memory, or in a file mapped into it.
 *
 * Which images are held is fixed at construction. Entries start empty and
 * are loaded from their files when regions are stored into them.
 *
 * Loading works in runs. A run is a block of consecutive held images of one
 * file. Every run of a file has the same number of images, except the last
 * one, which may have fewer. Storing any image loads its whole run in one
 * read of the file.
 */
class REXLIB_API buffer_image_scratch final
	: public image_scratch
{
public:
	/**
	 * @brief Construct a scratch for a list of locations.
	 *
	 * A location with a stack index names that index of the first axis of
	 * its file. A location without one names the whole file.
	 *
	 * Files are taken in the order the locations first name them. Each is
	 * opened once, to learn its shape and data type. If the buffer runs out
	 * of room, the current file keeps the lowest indices that fit and the
	 * remaining files are not opened.
	 *
	 * @param locations The images to hold.
	 * @param files Provider used to open the files.
	 * @param storage The buffer that stores the copies. Its size is the
	 * capacity of the scratch. It must be host accessible, and aligned for
	 * the data types of the files.
	 * @param run_length Number of held indices per run. A file that holds
	 * no more indices than this is a single run.
	 * @throws std::invalid_argument If @p storage is null, if it is not
	 * aligned for the data type of a file, or if @p run_length is zero.
	 * @throws std::out_of_range If a location has a stack index that its
	 * file does not have.
	 * @throws unsupported_capability_error If @p storage is not host
	 * accessible.
	 * @throws image_file_error If a file does not exist or can not be read.
	 * @throws unsupported_operation_error If no format can read a file.
	 * @throws image_format_error If a file is malformed or truncated.
	 */
	buffer_image_scratch(
		span<const image_location> locations,
		image_reader_provider &files,
		std::shared_ptr<buffer> storage,
		std::size_t run_length
	);

	~buffer_image_scratch() override;

	std::shared_ptr<image_scratch_entry>
	find(const std::string &path) override;

private:
	using entry_map_type =
		std::unordered_map<std::string, std::shared_ptr<image_scratch_entry>>;

	REXLIB_STD_MEMBER_INTERFACE
	entry_map_type m_entries;
};

} // namespace em
} // namespace rexlib
