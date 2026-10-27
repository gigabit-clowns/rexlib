// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/plans/index_operands.hpp>

#include <rexlib/core/layout/joint_layout_builder.hpp>

#include <sstream>
#include <stdexcept>
#include <vector>

namespace rexlib
{
namespace cpu
{

std::vector<std::ptrdiff_t> make_row_major_strides(
	span<const std::size_t> extents
)
{
	std::vector<std::ptrdiff_t> strides(extents.size());

	std::ptrdiff_t stride = 1;
	for (std::size_t axis = extents.size(); axis > 0; --axis)
	{
		strides[axis - 1] = stride;
		stride *= static_cast<std::ptrdiff_t>(extents[axis - 1]);
	}

	return strides;
}

std::vector<std::ptrdiff_t> make_row_major_strides(
	span<const std::size_t> extents,
	span<const std::size_t> counted_axes
)
{
	std::vector<bool> counted(extents.size(), false);
	for (const auto axis : counted_axes)
	{
		if (axis >= extents.size())
		{
			std::ostringstream oss;
			oss << "make_row_major_strides: Axis " << axis
				<< " is out of range for a rank of " << extents.size() << ".";
			throw std::out_of_range(oss.str());
		}
		counted[axis] = true;
	}

	std::vector<std::ptrdiff_t> strides(extents.size(), 0);

	std::ptrdiff_t stride = 1;
	for (std::size_t axis = extents.size(); axis > 0; --axis)
	{
		if (counted[axis - 1])
		{
			strides[axis - 1] = stride;
			stride *= static_cast<std::ptrdiff_t>(extents[axis - 1]);
		}
	}

	return strides;
}

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
	const auto strides = make_row_major_strides(extents);
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
