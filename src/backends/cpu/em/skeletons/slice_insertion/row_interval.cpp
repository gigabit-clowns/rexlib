// SPDX-License-Identifier: GPL-3.0-only

#include "row_interval.hpp"

#include <algorithm>
#include <cmath>

namespace rexlib
{
namespace cpu
{

row_interval::row_interval(std::ptrdiff_t begin, std::ptrdiff_t end) noexcept
	: m_begin(begin)
	, m_end(end)
{
}

std::ptrdiff_t row_interval::get_begin() const noexcept
{
	return m_begin;
}

std::ptrdiff_t row_interval::get_end() const noexcept
{
	return m_end;
}

bool row_interval::is_empty() const noexcept
{
	return m_end <= m_begin;
}

row_interval clip_row(
	const volume_frequency &base,
	const volume_frequency &step,
	const frequency_bounds &bounds,
	std::ptrdiff_t first,
	std::ptrdiff_t last
) noexcept
{
	auto low = static_cast<double>(first);
	auto high = static_cast<double>(last);

	for (std::size_t axis = 0; axis < 3; ++axis)
	{
		if (!std::isfinite(base[axis]) || !std::isfinite(step[axis]))
		{
			return row_interval(first, first);
		}

		const auto lower = bounds.get_lower(axis);
		const auto upper = bounds.get_upper(axis);
		if (step[axis] == 0.0)
		{
			if (!(lower <= base[axis] && base[axis] <= upper))
			{
				return row_interval(first, first);
			}
			continue;
		}

		const auto at_lower = (lower - base[axis]) / step[axis];
		const auto at_upper = (upper - base[axis]) / step[axis];
		low = std::max(low, std::min(at_lower, at_upper));
		high = std::min(high, std::max(at_lower, at_upper));
	}

	// Rounding outwards grants up to one sample at each end, so that it never
	// drops one.
	if (low > high)
	{
		return row_interval(first, first);
	}

	const auto begin = static_cast<std::ptrdiff_t>(std::floor(low));
	const auto end = static_cast<std::ptrdiff_t>(std::ceil(high)) + 1;
	return row_interval(
		std::max(begin, first),
		std::min(end, last + 1)
	);
}

row_interval bound_rows(
	const volume_frequency &x_step,
	const volume_frequency &y_step,
	const frequency_bounds &bounds,
	std::ptrdiff_t first_x,
	std::ptrdiff_t last_x,
	std::ptrdiff_t first_y,
	std::ptrdiff_t last_y
) noexcept
{
	auto low = static_cast<double>(first_y);
	auto high = static_cast<double>(last_y);

	for (std::size_t axis = 0; axis < 3; ++axis)
	{
		if (!std::isfinite(x_step[axis]) || !std::isfinite(y_step[axis]))
		{
			return row_interval(first_y, first_y);
		}

		// The part k_x x_step takes, over the whole row.
		const auto at_first = static_cast<double>(first_x) * x_step[axis];
		const auto at_last = static_cast<double>(last_x) * x_step[axis];
		const auto lower =
			bounds.get_lower(axis) - std::max(at_first, at_last);
		const auto upper =
			bounds.get_upper(axis) - std::min(at_first, at_last);

		if (y_step[axis] == 0.0)
		{
			if (!(lower <= 0.0 && 0.0 <= upper))
			{
				return row_interval(first_y, first_y);
			}
			continue;
		}

		const auto at_lower = lower / y_step[axis];
		const auto at_upper = upper / y_step[axis];
		low = std::max(low, std::min(at_lower, at_upper));
		high = std::min(high, std::max(at_lower, at_upper));
	}

	if (low > high)
	{
		return row_interval(first_y, first_y);
	}

	return row_interval(
		std::max(static_cast<std::ptrdiff_t>(std::floor(low)), first_y),
		std::min(static_cast<std::ptrdiff_t>(std::ceil(high)) + 1, last_y + 1)
	);
}

} // namespace cpu
} // namespace rexlib
