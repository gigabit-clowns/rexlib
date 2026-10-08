// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_region_bounds.hpp"

#include <em/image/formats/strided_transfer/image_region_offsets.hpp>

#include <vector>

namespace rexlib
{
namespace em
{
namespace tiff
{

void check_region_bounds(
	const image_transfer_plan &regions,
	const image_descriptor &file,
	span<const std::size_t> array_extents,
	span<const std::ptrdiff_t> array_strides,
	std::ptrdiff_t array_offset
)
{
	const auto file_extents = file.get_extents();
	std::vector<std::ptrdiff_t> file_strides(file_extents.size());

	std::ptrdiff_t stride = 1;
	for (auto axis = file_extents.size(); axis > 0; --axis)
	{
		file_strides[axis - 1] = stride;
		stride *= static_cast<std::ptrdiff_t>(file_extents[axis - 1]);
	}

	// Resolving the regions is what bounds checks them, and what they
	// resolve to is of no use here.
	image_region_offsets(
		regions,
		file_extents,
		make_span(file_strides.data(), file_strides.size()),
		array_extents,
		array_strides,
		array_offset
	);
}

} // namespace tiff
} // namespace em
} // namespace rexlib
