// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <memory>
#include <string>

namespace rexlib
{

class buffer;

/**
 * @brief Create a file and map it into host memory as a buffer.
 *
 * The file is created with the given size and mapped for reading and
 * writing. Any file already at @p path is replaced.
 *
 * The buffer is host memory, so it can be used wherever a host buffer can.
 * Its contents live in the file, which lets it hold more than the main
 * memory does. Writes reach the file when the operating system flushes
 * them.
 *
 * The buffer keeps no file open. The file is not removed when the buffer is
 * destroyed.
 *
 * @param path Path of the file to create.
 * @param size Size of the file in bytes.
 * @return std::shared_ptr<buffer> The buffer, of @p size bytes. Never null.
 * @throws std::invalid_argument If @p size is zero.
 * @throws file_error If the file can not be created, sized or mapped.
 */
REXLIB_API
std::shared_ptr<buffer> create_mapped_file_buffer(
	const std::string &path,
	std::size_t size
);

/**
 * @brief Map a file that already exists into host memory as a buffer.
 *
 * The whole file is mapped for reading and writing. Mapping it does not
 * change it: the file keeps its size and its contents, and other buffers
 * that map the same file stay valid.
 *
 * The buffer is otherwise like the one @ref create_mapped_file_buffer
 * returns. Buffers that map the same file share its contents.
 *
 * @param path Path of the file to map.
 * @return std::shared_ptr<buffer> The buffer, of the size of the file.
 * Never null.
 * @throws file_error If the file does not exist, is empty or can not be
 * mapped.
 */
REXLIB_API
std::shared_ptr<buffer> open_mapped_file_buffer(const std::string &path);

} // namespace rexlib
