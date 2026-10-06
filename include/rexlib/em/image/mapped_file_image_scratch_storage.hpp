// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <memory>
#include <string>

namespace rexlib
{
namespace em
{

class image_scratch_storage;

/**
 * @brief Map a file as storage for a scratch.
 *
 * The file is created if it is missing, given the size, and mapped for
 * reading and writing. A file already at @p path is kept: it is not
 * emptied, and its contents stay up to the new size. Storages that map the
 * same file share its contents.
 *
 * The contents of the storage live in the file, which lets it hold more
 * than the main memory does. Writes reach the file when the operating
 * system flushes them. The file is not removed when the storage is
 * destroyed.
 *
 * @param path Path of the file.
 * @param size Size of the file in bytes.
 * @return std::shared_ptr<image_scratch_storage> The storage, of @p size
 * bytes. Never null.
 * @throws image_file_error If @p size is zero, or if the file can not be
 * created, sized or mapped.
 */
REXLIB_API
std::shared_ptr<image_scratch_storage>
create_mapped_file_image_scratch_storage(
	const std::string &path,
	std::size_t size
);

} // namespace em
} // namespace rexlib
