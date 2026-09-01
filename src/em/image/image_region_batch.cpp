// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_region_batch.hpp>

#include <rexlib/core/platform/assert.hpp>

#include <stdexcept>

namespace rexlib
{
namespace em
{

image_region_batch::image_region_batch() noexcept
	: m_file_rank(0)
	, m_size(0)
{
}

image_region_batch::image_region_batch(
	const image_region_batch &other
) = default;
image_region_batch::image_region_batch(
	image_region_batch &&other
) noexcept = default;
image_region_batch::~image_region_batch() = default;

image_region_batch&
image_region_batch::operator=(const image_region_batch &other) = default;
image_region_batch&
image_region_batch::operator=(image_region_batch &&other) noexcept = default;

void image_region_batch::reset(
	span<const std::size_t> extents,
	std::size_t file_rank
)
{
	if (file_rank > extents.size())
	{
		throw std::invalid_argument(
			"image_region_batch::reset: The file rank exceeds the rank of "
			"the extents."
		);
	}

	const auto leading = extents.size() - file_rank;
	for (std::size_t i = 0; i < leading; ++i)
	{
		if (extents[i] != 1)
		{
			throw std::invalid_argument(
				"image_region_batch::reset: The extents preceding the ones of "
				"the file must all be one."
			);
		}
	}

	m_extents.assign(extents.begin(), extents.end());
	m_file_rank = file_rank;
	clear();
}

void image_region_batch::add(
	span<const std::size_t> file_offset,
	span<const std::size_t> array_offset
)
{
	if (file_offset.size() != m_file_rank)
	{
		throw std::invalid_argument(
			"image_region_batch::add: The file offset does not have the "
			"rank of the file."
		);
	}

	if (array_offset.size() != m_extents.size())
	{
		throw std::invalid_argument(
			"image_region_batch::add: The array offset does not have the rank "
			"of the extents."
		);
	}

	m_file_offsets.insert(
		m_file_offsets.end(),
		file_offset.begin(),
		file_offset.end()
	);
	m_array_offsets.insert(
		m_array_offsets.end(),
		array_offset.begin(),
		array_offset.end()
	);
	++m_size;
}

void image_region_batch::clear() noexcept
{
	m_file_offsets.clear();
	m_array_offsets.clear();
	m_size = 0;
}

void image_region_batch::reserve(std::size_t count)
{
	m_file_offsets.reserve(count * m_file_rank);
	m_array_offsets.reserve(count * m_extents.size());
}

std::size_t image_region_batch::get_size() const noexcept
{
	return m_size;
}

std::size_t image_region_batch::get_file_rank() const noexcept
{
	return m_file_rank;
}

std::size_t image_region_batch::get_array_rank() const noexcept
{
	return m_extents.size();
}

span<const std::size_t> image_region_batch::get_extents() const noexcept
{
	return make_span(m_extents.data(), m_extents.size());
}

span<const std::size_t>
image_region_batch::get_file_offset(std::size_t index) const noexcept
{
	REXLIB_ASSERT(index < m_size);
	return make_span(
		m_file_offsets.data() + (index * m_file_rank),
		m_file_rank
	);
}

span<const std::size_t>
image_region_batch::get_array_offset(std::size_t index) const noexcept
{
	REXLIB_ASSERT(index < m_size);
	const auto rank = m_extents.size();
	return make_span(m_array_offsets.data() + (index * rank), rank);
}

} // namespace em
} // namespace rexlib
