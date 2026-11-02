// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/reconstruction/insert_ctf_slices_operation.hpp>

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace rexlib
{
namespace em
{

namespace
{

double check_pixel_size(double pixel_size)
{
	if (!(std::isfinite(pixel_size) && pixel_size > 0.0))
	{
		std::ostringstream oss;
		oss << "insert_ctf_slices: the pixel size must be finite and "
			<< "positive, but it is " << pixel_size << ".";
		throw std::invalid_argument(oss.str());
	}
	return pixel_size;
}

} // anonymous namespace

insert_ctf_slices_operation::insert_ctf_slices_operation(
	const gridding_window &window,
	const ctf_optics &optics,
	double pixel_size
)
	: m_window(window)
	, m_optics(optics)
	, m_pixel_size(check_pixel_size(pixel_size))
{
}

const gridding_window&
insert_ctf_slices_operation::get_window() const noexcept
{
	return m_window;
}

const ctf_optics& insert_ctf_slices_operation::get_optics() const noexcept
{
	return m_optics;
}

double insert_ctf_slices_operation::get_pixel_size() const noexcept
{
	return m_pixel_size;
}

} // namespace em
} // namespace rexlib
