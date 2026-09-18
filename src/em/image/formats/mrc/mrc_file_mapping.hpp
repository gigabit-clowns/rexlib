// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/system/access_flags.hpp>

#include <cstddef>
#include <memory>
#include <string>

namespace boost
{
namespace interprocess
{
class file_mapping;
class mapped_region;
} // namespace interprocess
} // namespace boost

namespace rexlib
{
namespace em
{
namespace mrc
{

/**
 * @brief One MRC file, held in memory.
 *
 * The whole file is mapped from its first byte, header included. Where the
 * values begin is not a page boundary, and a mapping may only start at one,
 * so the header is mapped along with them and skipped by pointer arithmetic
 * instead.
 *
 * Mapping is what makes a file one strided operand like any other: a region
 * of it is read or written by the same loop that walks an array, with no
 * buffer in between and no per region system call.
 */
class mrc_file_mapping
{
public:
	/**
	 * @brief Map a file that already exists.
	 *
	 * @param path Path to the file.
	 * @param access Whether the mapping may be written through. Writing
	 * needs a file opened for both reading and writing.
	 * @throws image_format_error If the file can not be opened or mapped, or
	 * if it is empty.
	 */
	mrc_file_mapping(const std::string &path, access_flags access);

	mrc_file_mapping(const mrc_file_mapping &other) = delete;

	// Defined where boost::interprocess is included rather than defaulted
	// here: destroying either pointer needs the type it points at to be
	// complete, and neither is at this point.
	mrc_file_mapping(mrc_file_mapping &&other) noexcept;
	~mrc_file_mapping();

	mrc_file_mapping& operator=(const mrc_file_mapping &other) = delete;
	mrc_file_mapping& operator=(mrc_file_mapping &&other) noexcept;

	/**
	 * @brief Get the first byte of the file.
	 *
	 * @return byte* The mapped bytes. Writing through them is only defined
	 * when the mapping was opened for writing.
	 */
	byte* get_data() const noexcept;

	/**
	 * @brief Get how many bytes are mapped.
	 *
	 * @return std::size_t The size of the file.
	 */
	std::size_t get_size() const noexcept;

	/**
	 * @brief Make everything written through the mapping reach the storage.
	 *
	 * @throws image_format_error If the mapping could not be flushed.
	 */
	void flush();

private:
	std::unique_ptr<boost::interprocess::file_mapping> m_mapping;
	std::unique_ptr<boost::interprocess::mapped_region> m_region;
};

/**
 * @brief Create a file of a given size, replacing whatever is at that path.
 *
 * A mapping can neither create a file nor resize one, so a file being written
 * is laid out in full before it is mapped. The bytes it is laid out with are
 * unspecified.
 *
 * @param path Path to the file to create.
 * @param size Size of the file in bytes. Must not be zero.
 * @throws image_format_error If the file could not be created or sized.
 */
void create_file(const std::string &path, std::size_t size);

} // namespace mrc
} // namespace em
} // namespace rexlib
