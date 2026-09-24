// SPDX-License-Identifier: GPL-3.0-only

#include "image_region_read_plan.hpp"

#include "image_region_layout.hpp"

namespace rexlib
{
namespace em
{

image_region_read_plan::image_region_read_plan(
	const image_transfer_plan &regions,
	span<const std::size_t> file_extents,
	span<const std::ptrdiff_t> file_strides,
	span<const std::size_t> array_extents,
	span<const std::ptrdiff_t> array_strides,
	std::ptrdiff_t array_offset
)
	: m_offsets(
		regions,
		file_extents,
		file_strides,
		array_extents,
		array_strides,
		array_offset
	)
	, m_layout(build_region_layout(regions, array_strides, file_strides))
{
}

const image_region_offsets&
image_region_read_plan::get_offsets() const noexcept
{
	return m_offsets;
}

const joint_layout& image_region_read_plan::get_layout() const noexcept
{
	return m_layout;
}

} // namespace em
} // namespace rexlib
