// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <memory>
#include <string>

namespace rexlib
{

class completion;
class executor;

namespace em
{

class image_location;
class image_reader_provider;
class image_scratch_entry;

/**
 * @brief Copies of image files, kept where they are faster to reach than
 * the files themselves.
 *
 * A scratch holds one @ref image_scratch_entry for each file it keeps
 * something of, and finds it by the path to that file. What an entry keeps of
 * its file is the entry's implementation's own business.
 *
 * @par Thread safety
 * Every method may be called concurrently.
 */
class REXLIB_API image_scratch
{
public:
	image_scratch() noexcept;
	image_scratch(const image_scratch &other) = delete;
	image_scratch(image_scratch &&other) = delete;
	virtual ~image_scratch();

	image_scratch& operator=(const image_scratch &other) = delete;
	image_scratch& operator=(image_scratch &&other) = delete;

	/**
	 * @brief Get what is held of a file, to read it and to add to it.
	 *
	 * Shared ownership rather than a reference, so that an entry stays
	 * alive for as long as it is still read through.
	 *
	 * @param path Path to the file.
	 * @return std::shared_ptr<image_scratch_entry> The entry of the file, or
	 * null when nothing of it is held.
	 */
	virtual std::shared_ptr<image_scratch_entry>
	find(const std::string &path) = 0;

	/**
	 * @brief Get what is held of a file, to read it.
	 *
	 * @param path Path to the file.
	 * @return std::shared_ptr<const image_scratch_entry> The entry of the
	 * file, which may be read and not added to, or null when nothing of it
	 * is held.
	 */
	virtual std::shared_ptr<const image_scratch_entry>
	find(const std::string &path) const = 0;
};

/**
 * @brief Load what a list of locations names into a scratch,
 * asynchronously.
 *
 * A location with a stack index names that index of the first axis of its
 * file. A location without one names the whole file.
 *
 * One task is submitted for each file the locations name, in the order they
 * first name it. A task opens its file and stores what the locations name of
 * it into the entry of that file, which takes in what it has room for. A
 * file the scratch has no entry for is skipped.
 *
 * This function returns before the files are loaded. The scratch may be
 * read through in the meantime: a read loads what it needs and is not yet
 * loaded.
 *
 * @param scratch The scratch to load.
 * @param files Provider used to open the files. It must read the files
 * themselves, not read them through @p scratch.
 * @param executor Where the tasks run.
 * @param locations The images to load.
 * @return std::shared_ptr<completion> The completion, never null. It is
 * ready once every file is loaded or has failed, and it rethrows the first
 * failure.
 * @throws std::invalid_argument If @p files is null.
 */
REXLIB_API
std::shared_ptr<completion> prefetch_scratch_async(
	image_scratch &scratch,
	std::shared_ptr<image_reader_provider> files,
	rexlib::executor &executor,
	span<const image_location> locations
);

} // namespace em
} // namespace rexlib
