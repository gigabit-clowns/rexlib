// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <rexlib/core/platform/assert.hpp>

#include <stdexcept>

namespace rexlib
{
namespace em
{

namespace
{

std::vector<std::size_t> checked_extents(
	span<const std::size_t> extents,
	std::size_t file_rank,
	std::size_t array_rank
)
{
	if (extents.size() > file_rank)
	{
		throw std::invalid_argument(
			"image_transfer_plan: The regions do not fit in the rank of "
			"the file."
		);
	}

	if (extents.size() > array_rank)
	{
		throw std::invalid_argument(
			"image_transfer_plan: The regions do not fit in the rank of "
			"the array."
		);
	}

	return std::vector<std::size_t>(extents.begin(), extents.end());
}

} // namespace

image_transfer_plan::image_transfer_plan(
	span<const std::size_t> extents,
	std::size_t file_rank,
	std::size_t array_rank
)
	: m_extents(checked_extents(extents, file_rank, array_rank))
	, m_file_offsets(file_rank)
	, m_array_offsets(array_rank)
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

void image_transfer_plan::add(
	span<const std::size_t> file_offset,
	span<const std::size_t> array_offset
)
{
	if (file_offset.size() != m_file_offsets.get_rank())
	{
		throw std::invalid_argument(
			"image_transfer_plan::add: The file offset does not have the "
			"rank of the file."
		);
	}

	if (array_offset.size() != m_array_offsets.get_rank())
	{
		throw std::invalid_argument(
			"image_transfer_plan::add: The array offset does not have the "
			"rank of the array."
		);
	}

	m_file_offsets.add(file_offset);
	m_array_offsets.add(array_offset);
}

void image_transfer_plan::clear() noexcept
{
	m_file_offsets.clear();
	m_array_offsets.clear();
}

void image_transfer_plan::reserve(std::size_t count)
{
	m_file_offsets.reserve(count);
	m_array_offsets.reserve(count);
}

std::size_t image_transfer_plan::get_size() const noexcept
{
	return m_file_offsets.get_size();
}

std::size_t image_transfer_plan::get_rank() const noexcept
{
	return m_extents.size();
}

std::size_t image_transfer_plan::get_file_rank() const noexcept
{
	return m_file_offsets.get_rank();
}

std::size_t image_transfer_plan::get_array_rank() const noexcept
{
	return m_array_offsets.get_rank();
}

span<const std::size_t> image_transfer_plan::get_extents() const noexcept
{
	return make_span(m_extents.data(), m_extents.size());
}

span<const std::size_t>
image_transfer_plan::get_file_offset(std::size_t index) const noexcept
{
	return m_file_offsets.get(index);
}

span<const std::size_t>
image_transfer_plan::get_array_offset(std::size_t index) const noexcept
{
	return m_array_offsets.get(index);
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
