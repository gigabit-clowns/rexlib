// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>

#include <cstddef>

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief How one page of a TIFF file holds its samples.
 *
 * A page is an image of one sample per pixel, cut into blocks that are
 * encoded one by one and so decoded one by one: either strips, which span
 * the width of the page and a number of its rows, or tiles, which are
 * rectangles of one size laid out in a grid.
 *
 * Blocks are numbered by row and then by column of that grid. The last strip
 * holds only the rows the page has left, whereas a tile is always whole,
 * with the samples past the edge of the page unspecified.
 */
class tiff_page_layout
{
public:
	/**
	 * @brief Construct the layout of a page cut into strips.
	 *
	 * @param width Number of columns of the page.
	 * @param height Number of rows of the page.
	 * @param data_type Data type of one sample.
	 * @param rows_per_strip Number of rows every strip but the last one
	 * holds. Anything above @p height stands for a single strip.
	 * @return tiff_page_layout The layout.
	 * @throws std::invalid_argument If an extent or @p rows_per_strip is
	 * zero, or if @p data_type is unknown.
	 */
	static tiff_page_layout make_striped(
		std::size_t width,
		std::size_t height,
		numerical_type data_type,
		std::size_t rows_per_strip
	);

	/**
	 * @brief Construct the layout of a page cut into tiles.
	 *
	 * @param width Number of columns of the page.
	 * @param height Number of rows of the page.
	 * @param data_type Data type of one sample.
	 * @param tile_width Number of columns of one tile.
	 * @param tile_height Number of rows of one tile.
	 * @return tiff_page_layout The layout.
	 * @throws std::invalid_argument If an extent of the page or of a tile
	 * is zero, or if @p data_type is unknown.
	 */
	static tiff_page_layout make_tiled(
		std::size_t width,
		std::size_t height,
		numerical_type data_type,
		std::size_t tile_width,
		std::size_t tile_height
	);

	tiff_page_layout(const tiff_page_layout &other) = default;
	tiff_page_layout(tiff_page_layout &&other) noexcept = default;
	~tiff_page_layout() = default;

	tiff_page_layout& operator=(const tiff_page_layout &other) = default;
	tiff_page_layout&
	operator=(tiff_page_layout &&other) noexcept = default;

	/**
	 * @brief Get the number of columns of the page.
	 *
	 * @return std::size_t The width. Never zero.
	 */
	std::size_t get_width() const noexcept;

	/**
	 * @brief Get the number of rows of the page.
	 *
	 * @return std::size_t The height. Never zero.
	 */
	std::size_t get_height() const noexcept;

	/**
	 * @brief Get the data type of one sample.
	 *
	 * @return numerical_type The data type. Never unknown.
	 */
	numerical_type get_data_type() const noexcept;

	/**
	 * @brief Check whether the page is cut into tiles rather than strips.
	 *
	 * @return bool true if it is cut into tiles.
	 */
	bool is_tiled() const noexcept;

	/**
	 * @brief Get the number of columns of one block.
	 *
	 * @return std::size_t The width of a tile, or that of the page when it
	 * is cut into strips.
	 */
	std::size_t get_block_width() const noexcept;

	/**
	 * @brief Get the number of rows of one block.
	 *
	 * @return std::size_t The height of a tile, or the rows of every strip
	 * but the last one, which never exceed the height of the page.
	 */
	std::size_t get_block_height() const noexcept;

	/**
	 * @brief Get how many blocks lie side by side.
	 *
	 * @return std::size_t The columns of the grid of blocks, which is one
	 * for a page cut into strips.
	 */
	std::size_t get_blocks_across() const noexcept;

	/**
	 * @brief Get how many blocks lie one below another.
	 *
	 * @return std::size_t The rows of the grid of blocks.
	 */
	std::size_t get_blocks_down() const noexcept;

	/**
	 * @brief Get how many blocks the page is cut into.
	 *
	 * @return std::size_t The number of blocks.
	 */
	std::size_t get_block_count() const noexcept;

	/**
	 * @brief Get how many rows a block holds.
	 *
	 * @param block Index of the block. Must be below @ref get_block_count.
	 * @return std::size_t @ref get_block_height, save for the last strip of
	 * a page, which holds the rows left.
	 */
	std::size_t get_block_rows(std::size_t block) const noexcept;

	/**
	 * @brief Get how many bytes the samples of a block take once decoded.
	 *
	 * @param block Index of the block. Must be below @ref get_block_count.
	 * @return std::size_t The size in bytes.
	 */
	std::size_t get_block_size(std::size_t block) const noexcept;

	friend bool operator==(
		const tiff_page_layout &lhs,
		const tiff_page_layout &rhs
	) noexcept
	{
		return
			lhs.m_width == rhs.m_width &&
			lhs.m_height == rhs.m_height &&
			lhs.m_data_type == rhs.m_data_type &&
			lhs.m_block_width == rhs.m_block_width &&
			lhs.m_block_height == rhs.m_block_height &&
			lhs.m_tiled == rhs.m_tiled;
	}

	friend bool operator!=(
		const tiff_page_layout &lhs,
		const tiff_page_layout &rhs
	) noexcept
	{
		return !(lhs == rhs);
	}

private:
	tiff_page_layout(
		std::size_t width,
		std::size_t height,
		numerical_type data_type,
		std::size_t block_width,
		std::size_t block_height,
		bool tiled
	);

	std::size_t m_width;
	std::size_t m_height;
	numerical_type m_data_type;
	std::size_t m_block_width;
	std::size_t m_block_height;
	bool m_tiled;
};

} // namespace tiff
} // namespace em
} // namespace rexlib
