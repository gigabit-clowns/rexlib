// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_region_list.hpp>

#include <rexlib/core/platform/assert.hpp>

#include <stdexcept>

namespace rexlib
{
namespace em
{

image_region_list::image_region_list() noexcept
	: m_dataset_rank(0)
	, m_size(0)
{
}

image_region_list::image_region_list(
	const image_region_list &other
) = default;
image_region_list::image_region_list(
	image_region_list &&other
) noexcept = default;
image_region_list::~image_region_list() = default;

image_region_list&
image_region_list::operator=(const image_region_list &other) = default;
image_region_list&
image_region_list::operator=(image_region_list &&other) noexcept = default;

void image_region_list::reset(
	span<const std::size_t> extents,
	std::size_t dataset_rank
)
{
	if (dataset_rank > extents.size())
	{
		throw std::invalid_argument(
			"image_region_list::reset: The dataset rank exceeds the rank of "
			"the extents."
		);
	}

	const auto leading = extents.size() - dataset_rank;
	for (std::size_t i = 0; i < leading; ++i)
	{
		if (extents[i] != 1)
		{
			throw std::invalid_argument(
				"image_region_list::reset: The extents preceding the ones of "
				"the dataset must all be one."
			);
		}
	}

	m_extents.assign(extents.begin(), extents.end());
	m_dataset_rank = dataset_rank;
	clear();
}

void image_region_list::add(
	span<const std::size_t> dataset_offset,
	span<const std::size_t> array_offset
)
{
	if (dataset_offset.size() != m_dataset_rank)
	{
		throw std::invalid_argument(
			"image_region_list::add: The dataset offset does not have the "
			"rank of the dataset."
		);
	}

	if (array_offset.size() != m_extents.size())
	{
		throw std::invalid_argument(
			"image_region_list::add: The array offset does not have the rank "
			"of the extents."
		);
	}

	m_dataset_offsets.insert(
		m_dataset_offsets.end(),
		dataset_offset.begin(),
		dataset_offset.end()
	);
	m_array_offsets.insert(
		m_array_offsets.end(),
		array_offset.begin(),
		array_offset.end()
	);
	++m_size;
}

void image_region_list::clear() noexcept
{
	m_dataset_offsets.clear();
	m_array_offsets.clear();
	m_size = 0;
}

void image_region_list::reserve(std::size_t count)
{
	m_dataset_offsets.reserve(count * m_dataset_rank);
	m_array_offsets.reserve(count * m_extents.size());
}

std::size_t image_region_list::get_size() const noexcept
{
	return m_size;
}

std::size_t image_region_list::get_dataset_rank() const noexcept
{
	return m_dataset_rank;
}

std::size_t image_region_list::get_array_rank() const noexcept
{
	return m_extents.size();
}

span<const std::size_t> image_region_list::get_extents() const noexcept
{
	return make_span(m_extents.data(), m_extents.size());
}

span<const std::size_t>
image_region_list::get_dataset_offset(std::size_t index) const noexcept
{
	REXLIB_ASSERT(index < m_size);
	return make_span(
		m_dataset_offsets.data() + (index * m_dataset_rank),
		m_dataset_rank
	);
}

span<const std::size_t>
image_region_list::get_array_offset(std::size_t index) const noexcept
{
	REXLIB_ASSERT(index < m_size);
	const auto rank = m_extents.size();
	return make_span(m_array_offsets.data() + (index * rank), rank);
}

} // namespace em
} // namespace rexlib
