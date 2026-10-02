// SPDX-License-Identifier: GPL-3.0-only

#include "image_region_layout.hpp"

#include <rexlib/core/layout/joint_layout_builder.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

namespace rexlib
{
namespace em
{

namespace
{

span<const std::ptrdiff_t> get_region_strides(
	const image_transfer_shape &shape,
	span<const std::ptrdiff_t> strides
)
{
	return make_span(
		strides.data() + shape.get_leading_rank(strides.size()),
		shape.get_rank()
	);
}

} // anonymous namespace

joint_layout build_region_layout(
	const image_transfer_plan &regions,
	span<const std::ptrdiff_t> destination_strides,
	span<const std::ptrdiff_t> source_strides
)
{
	const auto &shape = regions.get_shape();
	const auto extents = shape.get_extents();

	joint_layout_builder builder;
	builder.set_extents(extents);
	builder.add_operand(
		extents,
		get_region_strides(shape, destination_strides),
		0
	);
	builder.add_operand(extents, get_region_strides(shape, source_strides), 0);
	return builder.build();
}

} // namespace em
} // namespace rexlib
