// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "tiff_file.hpp"
#include "tiff_page_layout.hpp"

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief Decodes the pages of a TIFF file whose pages are all alike.
 *
 * Every page of the file has the same size and data type, which is what
 * lets the file be taken for a stack. A page is decoded into memory laid out
 * row after row, whichever way the file cuts it into strips or tiles, and
 * only the blocks holding the rows asked for are decoded.
 *
 * The page decoded last is kept, along with which of its blocks have been
 * decoded, so that asking for more rows of it decodes only the blocks that
 * are still missing. Asking for another page drops it.
 *
 * A decoder is not safe to use from several threads at once.
 */
class tiff_page_decoder
{
public:
	/**
	 * @brief Open a file and check that its pages are all alike.
	 *
	 * @param path Path to the file.
	 * @throws image_file_error If the file can not be reached.
	 * @throws image_format_error If the file is not a TIFF file, if it has
	 * no pages, if a page is one this format can not transfer, or if its
	 * pages differ in size or data type.
	 */
	explicit tiff_page_decoder(const std::string &path);

	tiff_page_decoder(const tiff_page_decoder &other) = delete;
	tiff_page_decoder(tiff_page_decoder &&other) = delete;
	~tiff_page_decoder() = default;

	tiff_page_decoder& operator=(const tiff_page_decoder &other) = delete;
	tiff_page_decoder& operator=(tiff_page_decoder &&other) = delete;

	/**
	 * @brief Get how many pages the file holds.
	 *
	 * @return std::size_t The number of pages. Never zero.
	 */
	std::size_t get_page_count() const noexcept;

	/**
	 * @brief Get the number of columns of every page.
	 *
	 * @return std::size_t The width.
	 */
	std::size_t get_width() const noexcept;

	/**
	 * @brief Get the number of rows of every page.
	 *
	 * @return std::size_t The height.
	 */
	std::size_t get_height() const noexcept;

	/**
	 * @brief Get the data type of the samples of every page.
	 *
	 * @return numerical_type The data type.
	 */
	numerical_type get_data_type() const noexcept;

	/**
	 * @brief Decode a run of the rows of a page.
	 *
	 * @param page Index of the page. Must be below @ref get_page_count.
	 * @param first_row Index of the first row to decode.
	 * @param row_count How many rows to decode from there. The run must not
	 * reach past the page.
	 * @return const byte* The first sample of the page, with its rows one
	 * after another in the byte order of the host. The rows asked for hold
	 * their samples and the rest are unspecified. It refers to storage
	 * owned by this decoder, which the next call may overwrite.
	 * @throws image_format_error If the page can not be read, or if a block
	 * of it can not be decoded.
	 */
	const byte* decode(
		std::size_t page,
		std::size_t first_row,
		std::size_t row_count
	);

private:
	/**
	 * @brief Decode one block of the selected page into its place.
	 *
	 * @param block Index of the block.
	 */
	void decode_block(std::size_t block);

	std::string m_path;
	tiff_file m_file;
	std::size_t m_page_count;
	tiff_page_layout m_layout;
	std::size_t m_decoded_page;
	std::vector<bool> m_decoded_blocks;
	std::vector<byte> m_samples;
	std::vector<byte> m_tile;
};

} // namespace tiff
} // namespace em
} // namespace rexlib
