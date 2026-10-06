// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

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
};

} // namespace em
} // namespace rexlib
