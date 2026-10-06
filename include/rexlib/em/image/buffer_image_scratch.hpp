// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_scratch.hpp>

#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

namespace rexlib
{

class memory_allocator;

namespace em
{

class image_location;
class image_reader_provider;

/**
 * @brief A scratch that stores its copies in memory buffers.
 *
 * It holds the images that a list of locations names, up to a capacity in
 * bytes. Each file gets one entry and each entry gets one buffer from a
 * memory allocator. The allocator decides where the copies live.
 *
 * Which images are held is fixed at construction. Entries start empty and
 * are loaded from their files when regions are stored into them.
 *
 * Loading works in runs. A run is a block of consecutive held images of one
 * file. Storing any image loads its whole run in one read of the file.
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
	 * opened once, to learn its shape and data type. If the capacity runs
	 * out, the current file keeps the lowest indices that fit and the
	 * remaining files are not opened.
	 *
	 * @param locations The images to hold.
	 * @param files Provider used to open the files.
	 * @param allocator Allocator of the buffers. Its buffers must be host
	 * accessible.
	 * @param capacity Maximum number of bytes of image data to hold.
	 * @param run_size Maximum number of bytes of image data per run. A run
	 * holds at least one index. By default all the held indices of a file
	 * are one run.
	 * @throws std::out_of_range If a location has a stack index that its
	 * file does not have.
	 * @throws unsupported_capability_error If a buffer is not host
	 * accessible.
	 * @throws image_file_error If a file does not exist or can not be read.
	 * @throws unsupported_operation_error If no format can read a file.
	 * @throws image_format_error If a file is malformed or truncated.
	 */
	buffer_image_scratch(
		span<const image_location> locations,
		image_reader_provider &files,
		memory_allocator &allocator,
		std::size_t capacity,
		std::size_t run_size = std::numeric_limits<std::size_t>::max()
	);

	~buffer_image_scratch() override;

	std::shared_ptr<image_scratch_entry>
	find(const std::string &path) override;

	std::shared_ptr<const image_scratch_entry>
	find(const std::string &path) const override;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::unordered_map<std::string, std::shared_ptr<image_scratch_entry>>
		m_entries;
};

} // namespace em
} // namespace rexlib
