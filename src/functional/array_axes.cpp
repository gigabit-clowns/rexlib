// SPDX-License-Identifier: GPL-3.0-only

#include "array_axes.hpp"

#include <rexlib/core/layout/index.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>

#include <numeric>
#include <vector>

namespace rexlib
{

std::size_t get_rank(const const_array_ref &x)
{
	std::vector<std::size_t> extents;
	x.get_descriptor().get_layout().get_extents(extents);
	return extents.size();
}

ops::axis_list resolve_axes(
	const const_array_ref &x,
	span<const std::ptrdiff_t> axes
)
{
	const auto rank = get_rank(x);

	ops::axis_list result;
	result.reserve(axes.size());
	for (const auto axis : axes)
	{
		result.push_back(sanitize_index(axis, rank));
	}

	return result;
}

ops::axis_list every_axis(const const_array_ref &x)
{
	ops::axis_list result(get_rank(x));
	std::iota(result.begin(), result.end(), std::size_t(0));
	return result;
}

} // namespace rexlib
