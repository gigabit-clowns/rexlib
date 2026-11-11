// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/optics/ctf_1d_operation.hpp>

#include <em/ops/parameters/check_pixel_size.hpp>

namespace rexlib
{
namespace em
{

ctf_1d_operation::ctf_1d_operation(
	std::size_t extent,
	const ctf_optics &optics,
	double pixel_size
)
	: parametric_operation(extent)
	, m_optics(optics)
	, m_pixel_size(check_pixel_size("ctf_1d", pixel_size))
{
}

const ctf_optics& ctf_1d_operation::get_optics() const noexcept
{
	return m_optics;
}

double ctf_1d_operation::get_pixel_size() const noexcept
{
	return m_pixel_size;
}

} // namespace em
} // namespace rexlib
