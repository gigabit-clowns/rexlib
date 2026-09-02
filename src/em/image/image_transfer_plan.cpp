// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <rexlib/core/platform/assert.hpp>

#include <stdexcept>

namespace rexlib
{
namespace em
{

image_transfer_plan::image_transfer_plan() noexcept
	: m_source_rank(0)
	, m_destination_rank(0)
	, m_size(0)
{
}

image_transfer_plan::image_transfer_plan(
	const image_transfer_plan &other
) = default;
image_transfer_plan::image_transfer_plan(
	image_transfer_plan &&other
) noexcept = default;
image_transfer_plan::~image_transfer_plan() = default;

image_transfer_plan&
image_transfer_plan::operator=(const image_transfer_plan &other) = default;
image_transfer_plan&
image_transfer_plan::operator=(image_transfer_plan &&other) noexcept = default;

void image_transfer_plan::reset(
	span<const std::size_t> extents,
	std::size_t source_rank,
	std::size_t destination_rank
)
{
	if (extents.size() > source_rank)
	{
		throw std::invalid_argument(
			"image_transfer_plan::reset: The regions do not fit in the rank "
			"of the source."
		);
	}

	if (extents.size() > destination_rank)
	{
		throw std::invalid_argument(
			"image_transfer_plan::reset: The regions do not fit in the rank "
			"of the destination."
		);
	}

	m_extents.assign(extents.begin(), extents.end());
	m_source_rank = source_rank;
	m_destination_rank = destination_rank;
	clear();
}

void image_transfer_plan::add(
	span<const std::size_t> source_offset,
	span<const std::size_t> destination_offset
)
{
	if (source_offset.size() != m_source_rank)
	{
		throw std::invalid_argument(
			"image_transfer_plan::add: The source offset does not have the "
			"rank of the source."
		);
	}

	if (destination_offset.size() != m_destination_rank)
	{
		throw std::invalid_argument(
			"image_transfer_plan::add: The destination offset does not have "
			"the rank of the destination."
		);
	}

	m_source_offsets.insert(
		m_source_offsets.end(),
		source_offset.begin(),
		source_offset.end()
	);
	m_destination_offsets.insert(
		m_destination_offsets.end(),
		destination_offset.begin(),
		destination_offset.end()
	);
	++m_size;
}

void image_transfer_plan::clear() noexcept
{
	m_source_offsets.clear();
	m_destination_offsets.clear();
	m_size = 0;
}

void image_transfer_plan::reserve(std::size_t count)
{
	m_source_offsets.reserve(count * m_source_rank);
	m_destination_offsets.reserve(count * m_destination_rank);
}

std::size_t image_transfer_plan::get_size() const noexcept
{
	return m_size;
}

std::size_t image_transfer_plan::get_rank() const noexcept
{
	return m_extents.size();
}

std::size_t image_transfer_plan::get_source_rank() const noexcept
{
	return m_source_rank;
}

std::size_t image_transfer_plan::get_destination_rank() const noexcept
{
	return m_destination_rank;
}

span<const std::size_t> image_transfer_plan::get_extents() const noexcept
{
	return make_span(m_extents.data(), m_extents.size());
}

span<const std::size_t>
image_transfer_plan::get_source_offset(std::size_t index) const noexcept
{
	REXLIB_ASSERT(index < m_size);
	return make_span(
		m_source_offsets.data() + (index * m_source_rank),
		m_source_rank
	);
}

span<const std::size_t>
image_transfer_plan::get_destination_offset(std::size_t index) const noexcept
{
	REXLIB_ASSERT(index < m_size);
	return make_span(
		m_destination_offsets.data() + (index * m_destination_rank),
		m_destination_rank
	);
}

std::size_t get_region_extent(
	const image_transfer_plan &regions,
	std::size_t rank,
	std::size_t axis
) noexcept
{
	REXLIB_ASSERT(axis < rank);
	REXLIB_ASSERT(regions.get_rank() <= rank);

	const auto leading = rank - regions.get_rank();
	return axis < leading ? 1 : regions.get_extents()[axis - leading];
}

} // namespace em
} // namespace rexlib
