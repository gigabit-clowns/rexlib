// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_page_decoder.hpp"

#include <rexlib/core/span.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include <algorithm>

namespace rexlib
{
namespace em
{
namespace tiff
{

namespace
{

tiff_page_layout get_common_layout(
	tiff_file &file,
	std::size_t page_count,
	const std::string &path
)
{
	if (page_count == 0)
	{
		throw image_format_error(
			path + ": tiff_page_decoder: The file holds no page."
		);
	}

	const auto first = file.get_page_layout();
	for (std::size_t page = 1; page < page_count; ++page)
	{
		file.select_page(page);
		const auto layout = file.get_page_layout();
		if (layout.get_width() != first.get_width() ||
			layout.get_height() != first.get_height() ||
			layout.get_data_type() != first.get_data_type())
		{
			throw image_format_error(
				path + ": tiff_page_decoder: The pages of the file differ "
				"in size or data type."
			);
		}
	}

	return first;
}

} // anonymous namespace

tiff_page_decoder::tiff_page_decoder(const std::string &path)
	: m_path(path)
	, m_file(m_path, tiff_file_mode::read)
	, m_page_count(m_file.get_page_count())
	, m_layout(get_common_layout(m_file, m_page_count, m_path))
	, m_decoded_page(m_page_count)
	, m_decoded_blocks()
	, m_samples()
	, m_tile()
{
}

std::size_t tiff_page_decoder::get_page_count() const noexcept
{
	return m_page_count;
}

std::size_t tiff_page_decoder::get_width() const noexcept
{
	return m_layout.get_width();
}

std::size_t tiff_page_decoder::get_height() const noexcept
{
	return m_layout.get_height();
}

numerical_type tiff_page_decoder::get_data_type() const noexcept
{
	return m_layout.get_data_type();
}

const byte* tiff_page_decoder::decode(
	std::size_t page,
	std::size_t first_row,
	std::size_t row_count
)
{
	if (page != m_decoded_page)
	{
		// No page is held while this one is being selected, so that a
		// failure to select it does not leave the blocks of the previous
		// one standing for its own.
		m_decoded_page = m_page_count;
		m_file.select_page(page);
		m_layout = m_file.get_page_layout();
		m_decoded_blocks.assign(m_layout.get_block_count(), false);
		m_samples.resize(
			m_layout.get_width() * m_layout.get_height() *
			get_size(m_layout.get_data_type())
		);
		m_decoded_page = page;
	}

	if (row_count == 0)
	{
		return m_samples.data();
	}

	const auto block_height = m_layout.get_block_height();
	const auto blocks_across = m_layout.get_blocks_across();
	const auto first_block_row = first_row / block_height;
	const auto last_block_row = (first_row + row_count - 1) / block_height;
	for (auto row = first_block_row; row <= last_block_row; ++row)
	{
		for (std::size_t column = 0; column < blocks_across; ++column)
		{
			const auto block = row * blocks_across + column;
			if (!m_decoded_blocks[block])
			{
				decode_block(block);
				m_decoded_blocks[block] = true;
			}
		}
	}

	return m_samples.data();
}

void tiff_page_decoder::decode_block(std::size_t block)
{
	const auto element_size = get_size(m_layout.get_data_type());
	const auto row_size = m_layout.get_width() * element_size;
	const auto block_size = m_layout.get_block_size(block);
	const auto blocks_across = m_layout.get_blocks_across();
	const auto top = (block / blocks_across) * m_layout.get_block_height();

	if (!m_layout.is_tiled())
	{
		m_file.read_block(
			block,
			make_span(m_samples.data() + top * row_size, block_size)
		);
		return;
	}

	m_tile.resize(block_size);
	m_file.read_block(block, make_span(m_tile.data(), m_tile.size()));

	// A tile is whole even where it reaches past the page, so only the
	// part of it that lies on the page is kept.
	const auto left = (block % blocks_across) * m_layout.get_block_width();
	const auto rows =
		std::min(m_layout.get_block_height(), m_layout.get_height() - top);
	const auto columns =
		std::min(m_layout.get_block_width(), m_layout.get_width() - left);
	const auto tile_row_size = m_layout.get_block_width() * element_size;
	for (std::size_t row = 0; row < rows; ++row)
	{
		const auto *source = m_tile.data() + row * tile_row_size;
		std::copy(
			source,
			source + columns * element_size,
			m_samples.data() + (top + row) * row_size + left * element_size
		);
	}
}

} // namespace tiff
} // namespace em
} // namespace rexlib
