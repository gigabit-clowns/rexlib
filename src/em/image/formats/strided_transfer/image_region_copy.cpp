// SPDX-License-Identifier: GPL-3.0-only

#include "image_region_copy.hpp"

#include "image_host_access.hpp"
#include "image_region_read_walk.hpp"
#include "image_region_transfer.hpp"

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/memory/byte_order.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

void copy_regions(
	const_array_ref source,
	array_ref destination,
	const image_transfer_plan &regions
)
{
	const auto *source_storage =
		static_cast<const byte*>(get_host_data(source));
	auto *destination_data = get_host_data(destination);
	if (regions.get_region_count() == 0)
	{
		return;
	}

	const auto &source_descriptor = source.get_descriptor();
	std::vector<std::size_t> source_extents;
	std::vector<std::ptrdiff_t> source_strides;
	source_descriptor.get_layout().get_extents(source_extents);
	source_descriptor.get_layout().get_strides(source_strides);

	const auto &destination_descriptor = destination.get_descriptor();
	std::vector<std::size_t> destination_extents;
	std::vector<std::ptrdiff_t> destination_strides;
	destination_descriptor.get_layout().get_extents(destination_extents);
	destination_descriptor.get_layout().get_strides(destination_strides);

	const image_region_read_walk walk(
		regions,
		make_span(source_extents),
		make_span(source_strides),
		make_span(destination_extents),
		make_span(destination_strides),
		destination_descriptor.get_layout().get_offset()
	);

	const auto source_type = source_descriptor.get_data_type();
	const auto element_size =
		static_cast<std::ptrdiff_t>(get_size(source_type));
	const auto *source_data =
		source_storage +
		source_descriptor.get_layout().get_offset() * element_size;

	read_regions(
		walk,
		destination_data,
		destination_descriptor.get_data_type(),
		source_data,
		source_type,
		get_system_byte_order()
	);
}

} // namespace em
} // namespace rexlib
