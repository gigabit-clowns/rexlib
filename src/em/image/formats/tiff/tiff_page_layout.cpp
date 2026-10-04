// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_page_layout.hpp"

#include <algorithm>
#include <stdexcept>

namespace rexlib
{
namespace em
{
namespace tiff
{

namespace
{

std::size_t count_blocks(std::size_t extent, std::size_t block) noexcept
{
	return (extent + block - 1) / block;
}

} // anonymous namespace

tiff_page_layout tiff_page_layout::make_striped(
	std::size_t width,
	std::size_t height,
	numerical_type data_type,
	std::size_t rows_per_strip
)
{
	return tiff_page_layout(
		width,
		height,
		data_type,
		width,
		std::min(rows_per_strip, height),
		false
	);
}

tiff_page_layout tiff_page_layout::make_tiled(
	std::size_t width,
	std::size_t height,
	numerical_type data_type,
	std::size_t tile_width,
	std::size_t tile_height
)
{
	return tiff_page_layout(
		width,
		height,
		data_type,
		tile_width,
		tile_height,
		true
	);
}

tiff_page_layout::tiff_page_layout(
	std::size_t width,
	std::size_t height,
	numerical_type data_type,
	std::size_t block_width,
	std::size_t block_height,
	bool tiled
)
	: m_width(width)
	, m_height(height)
	, m_data_type(data_type)
	, m_block_width(block_width)
	, m_block_height(block_height)
	, m_tiled(tiled)
{
	if (m_width == 0 || m_height == 0)
	{
		throw std::invalid_argument(
			"tiff_page_layout: A page has no extent of zero."
		);
	}

	if (m_block_width == 0 || m_block_height == 0)
	{
		throw std::invalid_argument(
			"tiff_page_layout: A block has no extent of zero."
		);
	}

	if (m_data_type == numerical_type::unknown)
	{
		throw std::invalid_argument(
			"tiff_page_layout: The data type is unknown."
		);
	}
}

std::size_t tiff_page_layout::get_width() const noexcept
{
	return m_width;
}

std::size_t tiff_page_layout::get_height() const noexcept
{
	return m_height;
}

numerical_type tiff_page_layout::get_data_type() const noexcept
{
	return m_data_type;
}

bool tiff_page_layout::is_tiled() const noexcept
{
	return m_tiled;
}

std::size_t tiff_page_layout::get_block_width() const noexcept
{
	return m_block_width;
}

std::size_t tiff_page_layout::get_block_height() const noexcept
{
	return m_block_height;
}

std::size_t tiff_page_layout::get_blocks_across() const noexcept
{
	return count_blocks(m_width, m_block_width);
}

std::size_t tiff_page_layout::get_blocks_down() const noexcept
{
	return count_blocks(m_height, m_block_height);
}

std::size_t tiff_page_layout::get_block_count() const noexcept
{
	return get_blocks_across() * get_blocks_down();
}

std::size_t tiff_page_layout::get_block_rows(std::size_t block) const noexcept
{
	if (m_tiled)
	{
		return m_block_height;
	}

	return std::min(m_block_height, m_height - block * m_block_height);
}

std::size_t tiff_page_layout::get_block_size(std::size_t block) const noexcept
{
	return get_block_rows(block) * m_block_width * get_size(m_data_type);
}

} // namespace tiff
} // namespace em
} // namespace rexlib
