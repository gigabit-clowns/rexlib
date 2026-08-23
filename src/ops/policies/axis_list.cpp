// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/policies/axis_list.hpp>

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace rex
{
namespace ops
{

axis_list make_sorted_axis_list(axis_list axes)
{
	std::sort(axes.begin(), axes.end());

	const auto repeated = std::adjacent_find(axes.begin(), axes.end());
	if (repeated != axes.end())
	{
		std::ostringstream oss;
		oss << "Axis " << *repeated << " is named more than once.";
		throw std::invalid_argument(oss.str());
	}

	return axes;
}

void check_axes_within_rank(
	const operation_descriptor &descriptor,
	span<const std::size_t> axes,
	std::size_t rank
)
{
	// Sorted, so only the last one can be out of range.
	if (!axes.empty() && axes.back() >= rank)
	{
		std::ostringstream oss;
		oss << descriptor << ": axis " << axes.back()
			<< " was named, but the operands only have " << rank
			<< " of them.";
		throw std::out_of_range(oss.str());
	}
}

} // namespace ops
} // namespace rex
