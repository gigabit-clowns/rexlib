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
 * @brief Create a file and map it as storage for a scratch.
 *
 * The file is created with the given size and mapped for reading and
 * writing. Any file already at @p path is replaced.
 *
 * The contents of the storage live in the file, which lets it hold more
 * than the main memory does. Writes reach the file when the operating
 * system flushes them. The file is not removed when the storage is
 * destroyed.
 *
 * @param path Path of the file to create.
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

/**
 * @brief Map a file that already exists as storage for a scratch.
 *
 * The whole file is mapped for reading and writing. Mapping it does not
 * change it: the file keeps its size and its contents, and other storage
 * that maps the same file stays valid.
 *
 * The storage is otherwise like the one
 * @ref create_mapped_file_image_scratch_storage returns. Storages that map
 * the same file share its contents.
 *
 * @param path Path of the file to map.
 * @return std::shared_ptr<image_scratch_storage> The storage, of the size
 * of the file. Never null.
 * @throws image_file_error If the file does not exist, is empty or can not
 * be mapped.
 */
REXLIB_API
std::shared_ptr<image_scratch_storage>
open_mapped_file_image_scratch_storage(const std::string &path);

} // namespace em
} // namespace rexlib
