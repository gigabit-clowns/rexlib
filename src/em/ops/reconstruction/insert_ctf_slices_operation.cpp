// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/reconstruction/insert_ctf_slices_operation.hpp>

#include <em/ops/parameters/check_pixel_size.hpp>

namespace rexlib
{
namespace em
{

insert_ctf_slices_operation::insert_ctf_slices_operation(
	const gridding_window &window,
	const ctf_optics &optics,
	double pixel_size
)
	: m_window(window)
	, m_optics(optics)
	, m_pixel_size(check_pixel_size("insert_ctf_slices", pixel_size))
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
