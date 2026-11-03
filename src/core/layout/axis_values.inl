// SPDX-License-Identifier: GPL-3.0-only

#include "axis_values.hpp"

namespace rexlib
{

template <typename T>
inline
std::vector<typename std::remove_const<T>::type> select_axes(
	span<T> values,
	span<const std::size_t> axes
)
{
	std::vector<typename std::remove_const<T>::type> result;
	result.reserve(axes.size());
	for (const auto axis : axes)
	{
		result.push_back(values[axis]);
	}
	return result;
}

} // namespace rexlib
