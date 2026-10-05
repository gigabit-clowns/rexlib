// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "tiff_compression.hpp"
#include "tiff_file_mode.hpp"
#include "tiff_page_layout.hpp"

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/span.hpp>

#include <tiffio.h>

#include <cstddef>
#include <string>

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief One TIFF file, opened through libtiff.
 *
 * A file is a sequence of pages, one of which is selected at a time: what
 * is asked of the file is asked of that page. A file opened for reading
 * starts at its first page, and one being created gains a page with every
 * @ref write_page.
 *
 * What libtiff reports as an error is raised as an exception carrying its
 * message and the path of the file, and what it reports as a warning is
 * logged and otherwise ignored. Neither reaches the handlers libtiff keeps
 * for the whole process.
 *
 * A file is not safe to use from several threads at once.
 */
class tiff_file
{
public:
	/**
	 * @brief Open a file, or create one.
	 *
	 * Creating a file replaces whatever is at that path.
	 *
	 * @param path Path to the file.
	 * @param mode What the file is opened for.
	 * @throws image_file_error If the file can not be reached or created.
	 * @throws image_format_error If the file is reached but is not a TIFF
	 * file libtiff can open.
	 */
	tiff_file(const std::string &path, tiff_file_mode mode);

	tiff_file(const tiff_file &other) = delete;
	tiff_file(tiff_file &&other) = delete;
	~tiff_file();

	tiff_file& operator=(const tiff_file &other) = delete;
	tiff_file& operator=(tiff_file &&other) = delete;

	/**
	 * @brief Get how many pages the file holds.
	 *
	 * @return std::size_t The number of pages.
	 */
	std::size_t get_page_count();

	/**
	 * @brief Select the page the file is asked about from here on.
	 *
	 * @param page Index of the page.
	 * @throws image_format_error If the file has no such page, or if the
	 * page can not be read.
	 */
	void select_page(std::size_t page);

	/**
	 * @brief Get how the selected page holds its samples.
	 *
	 * @return tiff_page_layout The layout of the page.
	 * @throws image_format_error If the page does not state its size, if it
	 * holds anything but one sample per pixel, if its samples have no
	 * counterpart among the data types this format transfers, or if they are
	 * compressed with a scheme this build of libtiff can not decode.
	 */
	tiff_page_layout get_page_layout();

	/**
	 * @brief Decode one block of the selected page.
	 *
	 * The samples come in the byte order of the host, whichever one the file
	 * states them in.
	 *
	 * @param block Index of the block, as @ref tiff_page_layout numbers
	 * them.
	 * @param destination Where the samples are written. Its size must be
	 * that @ref tiff_page_layout::get_block_size gives for @p block.
	 * @throws image_format_error If the block can not be decoded, or does
	 * not hold as many samples as @p destination takes.
	 */
	void read_block(std::size_t block, span<byte> destination);

	/**
	 * @brief Append a page to a file being created.
	 *
	 * @param layout How the page holds its samples. Must be cut into strips.
	 * @param compression How the samples are encoded.
	 * @param samples The samples of the page, row after row, in the byte
	 * order of the host. Their size must be that of a page of @p layout.
	 * They are taken as writable because libtiff is handed them as such:
	 * it reorders the bytes of the samples in place when a file is not in
	 * the byte order of the host. A file created here is, so they are left
	 * as they are.
	 * @throws std::invalid_argument If @p layout is cut into tiles, or if
	 * @p samples do not have the size of the page.
	 * @throws unsupported_operation_error If no sample holds the data type
	 * of @p layout.
	 * @throws image_file_error If the page can not be written.
	 */
	void write_page(
		const tiff_page_layout &layout,
		tiff_compression compression,
		span<byte> samples
	);

private:
	std::string m_path;
	std::string m_last_error;
	TIFF *m_handle;
};

} // namespace tiff
} // namespace em
} // namespace rexlib
