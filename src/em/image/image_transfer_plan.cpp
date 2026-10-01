// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{

image_transfer_plan::image_transfer_plan(image_transfer_shape shape)
	: m_shape(std::move(shape))
	, m_file_offsets(m_shape.get_file_rank())
	, m_array_offsets(m_shape.get_array_rank())
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

std::size_t image_transfer_plan::get_region_count() const noexcept
{
	return m_file_offsets.get_index_count();
}

const image_transfer_shape& image_transfer_plan::get_shape() const noexcept
{
	return m_shape;
}

span<const std::size_t>
image_transfer_plan::get_file_offset(std::size_t region_index) const noexcept
{
	return m_file_offsets.get(region_index);
}

span<const std::size_t>
image_transfer_plan::get_array_offset(std::size_t region_index) const noexcept
{
	return m_array_offsets.get(region_index);
}

} // namespace em
} // namespace rexlib
