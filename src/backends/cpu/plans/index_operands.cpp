// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/plans/index_operands.hpp>

#include <rexlib/core/layout/joint_layout_builder.hpp>

#include <vector>

namespace rexlib
{
namespace cpu
{

void add_index_operands(
	joint_layout_builder& /*builder*/,
	span<const std::size_t> /*extents*/,
	no_index_tag
) noexcept
{
	// No index to be added.
}

void add_index_operands(
	joint_layout_builder &builder,
	span<const std::size_t> extents,
	linear_index_tag
)
{
	std::vector<std::ptrdiff_t> strides(extents.size());

	std::ptrdiff_t stride = 1;
	for (std::size_t axis = extents.size(); axis > 0; --axis)
	{
		strides[axis - 1] = stride;
		stride *= static_cast<std::ptrdiff_t>(extents[axis - 1]);
	}

	builder.add_operand(extents, make_span(strides), 0);
}

void add_index_operands(
	joint_layout_builder &builder,
	span<const std::size_t> extents,
	multidimensional_index_tag
)
{
	std::vector<std::ptrdiff_t> strides(extents.size(), 0);
	for (std::size_t axis = 0; axis < extents.size(); ++axis)
	{
		strides[axis] = 1;
		builder.add_operand(extents, make_span(strides), 0);
		strides[axis] = 0;
	}
}

} // namespace cpu
} // namespace rexlib
