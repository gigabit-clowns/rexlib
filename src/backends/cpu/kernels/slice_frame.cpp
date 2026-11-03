// SPDX-License-Identifier: GPL-3.0-only

#include "slice_frame.hpp"

namespace rexlib
{
namespace cpu
{

slice_frame::slice_frame(
	const volume_frequency &x_row,
	const volume_frequency &y_row,
	double scale
) noexcept
	: m_x_step{{ x_row[0] * scale, x_row[1] * scale, x_row[2] * scale }}
	, m_y_step{{ y_row[0] * scale, y_row[1] * scale, y_row[2] * scale }}
{
}

const volume_frequency& slice_frame::get_x_step() const noexcept
{
	return m_x_step;
}

const volume_frequency& slice_frame::get_y_step() const noexcept
{
	return m_y_step;
}

volume_frequency slice_frame::map(double k_x, double k_y) const noexcept
{
	return {{
		k_x * m_x_step[0] + k_y * m_y_step[0],
		k_x * m_x_step[1] + k_y * m_y_step[1],
		k_x * m_x_step[2] + k_y * m_y_step[2]
	}};
}

} // namespace cpu
} // namespace rexlib
