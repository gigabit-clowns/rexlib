// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_page_regions.hpp"

#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>

namespace rexlib
{
namespace em
{
namespace tiff
{

namespace
{

const std::size_t page_rank = 2;
const std::size_t stack_rank = 3;

image_transfer_shape make_page_shape(const image_transfer_shape &shape)
{
	const auto extents = shape.get_extents();
	const auto rank = std::min(extents.size(), page_rank);

	return image_transfer_shape(
		std::vector<std::size_t>(extents.end() - rank, extents.end()),
		page_rank,
		shape.get_array_rank()
	);
}

} // anonymous namespace

tiff_page_regions::tiff_page_regions(const image_transfer_plan &regions)
{
	const auto &shape = regions.get_shape();
	const auto file_rank = shape.get_file_rank();
	if (file_rank != page_rank && file_rank != stack_rank)
	{
		throw std::invalid_argument(
			"tiff_page_regions: The regions are stated against neither a "
			"page nor a stack of them."
		);
	}

	const bool stacked = file_rank == stack_rank;
	const bool spanning = shape.get_rank() == stack_rank;
	const auto pages_per_region = spanning ? shape.get_extents()[0] : 1;
	const auto region_count = regions.get_region_count();

	std::set<std::size_t> reached;
	for (std::size_t region = 0; region < region_count; ++region)
	{
		const auto first = stacked ? regions.get_file_offset(region)[0] : 0;
		for (std::size_t page = 0; page < pages_per_region; ++page)
		{
			reached.insert(first + page);
		}
	}

	const auto page_shape = make_page_shape(shape);
	const auto rows = page_shape.get_extent(page_rank, 0);
	m_pages.assign(reached.cbegin(), reached.cend());
	m_regions.assign(m_pages.size(), image_transfer_plan(page_shape));
	m_first_rows.assign(
		m_pages.size(), std::numeric_limits<std::size_t>::max());
	m_end_rows.assign(m_pages.size(), 0);

	for (std::size_t region = 0; region < region_count; ++region)
	{
		const auto file_offset = regions.get_file_offset(region);
		const auto page_offset = make_span(
			file_offset.data() + (file_rank - page_rank), page_rank);
		const auto array_span = regions.get_array_offset(region);
		std::vector<std::size_t> array_offset(
			array_span.begin(), array_span.end());

		const auto first = stacked ? file_offset[0] : 0;
		const auto row = page_offset[0];
		for (std::size_t page = 0; page < pages_per_region; ++page)
		{
			const auto position = static_cast<std::size_t>(
				std::lower_bound(
					m_pages.cbegin(), m_pages.cend(), first + page
				) - m_pages.cbegin()
			);

			m_regions[position].add(
				page_offset,
				make_span(array_offset.data(), array_offset.size())
			);
			m_first_rows[position] = std::min(m_first_rows[position], row);
			m_end_rows[position] =
				std::max(m_end_rows[position], row + rows);

			if (spanning)
			{
				++array_offset[array_offset.size() - stack_rank];
			}
		}
	}
}

std::size_t tiff_page_regions::get_page_count() const noexcept
{
	return m_pages.size();
}

std::size_t tiff_page_regions::get_page(std::size_t position) const noexcept
{
	return m_pages[position];
}

const image_transfer_plan&
tiff_page_regions::get_regions(std::size_t position) const noexcept
{
	return m_regions[position];
}

std::size_t
tiff_page_regions::get_first_row(std::size_t position) const noexcept
{
	return m_first_rows[position];
}

std::size_t
tiff_page_regions::get_row_count(std::size_t position) const noexcept
{
	return m_end_rows[position] - m_first_rows[position];
}

} // namespace tiff
} // namespace em
} // namespace rexlib
