// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/image/image_scratch.hpp>
#include <rexlib/em/image/image_scratch_open_mode.hpp>

#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

namespace rexlib
{
namespace em
{

class image_location_grouping;
class image_reader_provider;
class image_scratch_storage;

/**
 * @brief A scratch that holds the images that image_location-s can name, each
 * in a slot of its storage.
 *
 * What it holds of a file are indices of the first axis of the file, which
 * is what an @ref image_location names: for example the images of a stack.
 * Each held index has a slot in the storage.
 *
 * It holds the images that a grouping of locations names, as far as they
 * fit in the storage. Each file gets one entry, and the entries are stored
 * one after another. The storage decides where the copies live: in main
 * memory, or in a file mapped into it.
 *
 * Which images are held is fixed at construction. Entries are loaded from
 * their files when regions are stored into them.
 *
 * Loading works in runs. A run is a block of consecutive held images of one
 * file. Every run of a file has the same number of images, except the last
 * one, which may have fewer. Storing any image loads its whole run in one
 * read of the file.
 *
 * The storage also records what is loaded, so that a scratch constructed
 * later over the same storage can continue from it. The storage starts with
 * a fingerprint of how the scratch is laid out: the paths and the held
 * indices, the run length, and the data type and extents of each file.
 * Each run has a flag, which holds the modification time that its file had
 * when the run was loaded.
 */
class REXLIB_API indexed_image_scratch final
	: public image_scratch
{
public:
	/**
	 * @brief Construct a scratch for a grouping of locations.
	 *
	 * A location with a stack index names that index of the first axis of
	 * its file. A location without one names the whole file.
	 *
	 * Files are taken in the order of the grouping, which is ascending
	 * order of path. Each is opened once, to learn its shape and data type.
	 * If the storage runs out of room, the current file keeps the lowest
	 * indices that fit and the remaining files are not opened.
	 *
	 * @param locations The images to hold, grouped by file.
	 * @param files Provider used to open the files.
	 * @param storage Where the copies are stored. Its size is the capacity
	 * of the scratch. It must be aligned for 64-bit integers and for the
	 * data types of the files.
	 * @param run_length Number of held indices per run. A file that holds
	 * no more indices than this is a single run.
	 * @param mode Whether the scratch starts empty or resumed.
	 * - An empty scratch has nothing loaded, and does not read @p storage.
	 * - A resumed scratch keeps the runs that an earlier scratch loaded
	 * into @p storage, if that scratch had the same fingerprint. A run is
	 * kept only if its file has not been modified since. A file whose
	 * modification time can not be read is never kept. If the fingerprint
	 * differs, the scratch starts empty. The contents of @p storage must be
	 * defined, which those of newly allocated memory are not.
	 * @throws std::invalid_argument If @p storage is null, if it is smaller
	 * than a fingerprint, if it is not aligned for 64-bit integers or for
	 * the data type of a file, or if @p run_length is zero.
	 * @throws std::out_of_range If a location has a stack index that its
	 * file does not have.
	 * @throws image_file_error If a file does not exist or can not be read.
	 * @throws unsupported_operation_error If no format can read a file.
	 * @throws image_format_error If a file is malformed or truncated.
	 */
	indexed_image_scratch(
		const image_location_grouping &locations,
		image_reader_provider &files,
		std::shared_ptr<image_scratch_storage> storage,
		std::size_t run_length,
		image_scratch_open_mode mode = image_scratch_open_mode::empty
	);

	~indexed_image_scratch() override;

	std::shared_ptr<image_scratch_entry>
	find(const std::string &path) override;

private:
	using entry_map_type =
		std::unordered_map<std::string, std::shared_ptr<image_scratch_entry>>;

	REXLIB_STD_MEMBER_INTERFACE
	entry_map_type m_entries;
};

/**
 * @brief Create a scratch that stores its copies in main memory.
 *
 * The memory is as large as the scratch needs for the images it holds. The
 * scratch holds every image of @p locations, unless that needs more than
 * @p max_size. It then holds the images that fit, as
 * @ref indexed_image_scratch describes.
 *
 * Each file is opened twice through @p files: once to compute the size and
 * once to construct the scratch.
 *
 * @param locations The images to hold, grouped by file.
 * @param files Provider used to open the files.
 * @param run_length Number of held indices per run.
 * @param max_size Maximum number of bytes to allocate. By default there
 * is no maximum.
 * @return std::shared_ptr<image_scratch> The scratch. Never null.
 * @throws std::invalid_argument If @p run_length is zero, or if there is
 * nothing to hold: @p locations names no image, or @p max_size has no room
 * for one.
 * @throws std::bad_alloc If the memory can not be allocated.
 * @throws std::out_of_range If a location has a stack index that its
 * file does not have.
 * @throws image_file_error If a file does not exist or can not be read.
 * @throws unsupported_operation_error If no format can read a file.
 * @throws image_format_error If a file is malformed or truncated.
 */
REXLIB_API
std::shared_ptr<image_scratch> create_host_image_scratch(
	const image_location_grouping &locations,
	image_reader_provider &files,
	std::size_t run_length,
	std::size_t max_size = std::numeric_limits<std::size_t>::max()
);

/**
 * @brief Create a scratch that stores its copies in a file.
 *
 * The file is mapped into memory, and is as large as the scratch needs for
 * the images it holds. The scratch holds every image of @p locations,
 * unless that needs more than @p max_size. It then holds the images that
 * fit, as @ref indexed_image_scratch describes.
 *
 * If the file at @p path already holds a scratch with the same
 * fingerprint, the scratch resumes from what that file has loaded.
 * Programs that hold the same images may use the file at the same time,
 * and share it.
 *
 * Otherwise the file is overwritten: it is created if it is missing, given
 * the size of the scratch, and the scratch starts empty. A warning is
 * logged if the file held something else. Programs that use the file at
 * the same time must therefore hold the same images.
 *
 * The file is not removed when the scratch is destroyed.
 *
 * Each image file is opened twice through @p files: once to compute the
 * size and once to construct the scratch.
 *
 * @param locations The images to hold, grouped by file.
 * @param files Provider used to open the image files.
 * @param path Path of the scratch file.
 * @param run_length Number of held indices per run.
 * @param max_size Maximum size of the file in bytes. By default there is no
 * maximum.
 * @return std::shared_ptr<image_scratch> The scratch. Never null.
 * @throws std::invalid_argument If @p run_length is zero, or if there is
 * nothing to hold: @p locations names no image, or @p max_size has no room
 * for one. No file is created then.
 * @throws std::out_of_range If a location has a stack index that its
 * file does not have.
 * @throws image_file_error If the scratch file can not be created, sized
 * or mapped, or if an image file does not exist or can not be read.
 * @throws unsupported_operation_error If no format can read an image file.
 * @throws image_format_error If an image file is malformed or truncated.
 */
REXLIB_API
std::shared_ptr<image_scratch> create_mapped_file_image_scratch(
	const image_location_grouping &locations,
	image_reader_provider &files,
	const std::string &path,
	std::size_t run_length,
	std::size_t max_size = std::numeric_limits<std::size_t>::max()
);

} // namespace em
} // namespace rexlib
