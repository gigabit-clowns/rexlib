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

span<const std::ptrdiff_t>
trailing(span<const std::ptrdiff_t> strides, std::size_t count)
{
	return make_span(strides.data() + (strides.size() - count), count);
}

} // anonymous namespace

joint_layout build_region_layout(
	const image_transfer_plan &regions,
	span<const std::ptrdiff_t> destination_strides,
	span<const std::ptrdiff_t> source_strides
)
{
	const auto rank = regions.get_rank();
	const auto extents = regions.get_extents();

	joint_layout_builder builder;
	builder.set_extents(extents);
	builder.add_operand(extents, trailing(destination_strides, rank), 0);
	builder.add_operand(extents, trailing(source_strides, rank), 0);
	return builder.build();
}

} // namespace em
} // namespace rexlib
