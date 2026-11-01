// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/parameters/gridding_window.hpp>

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace rexlib
{
namespace em
{

gridding_window gridding_window::nearest() noexcept
{
	return gridding_window(gridding_profile::box, 0.5, 0.0);
}

gridding_window gridding_window::linear() noexcept
{
	return gridding_window(gridding_profile::triangle, 1.0, 0.0);
}

gridding_window gridding_window::kaiser_bessel(
	double half_width,
	double shape
)
{
	if (!(std::isfinite(half_width) && half_width > 0.0))
	{
		std::ostringstream oss;
		oss << "gridding_window: the half-width of a Kaiser-Bessel window "
			<< "must be finite and positive, but it is " << half_width << ".";
		throw std::invalid_argument(oss.str());
	}
	if (!(std::isfinite(shape) && shape >= 0.0))
	{
		std::ostringstream oss;
		oss << "gridding_window: the shape of a Kaiser-Bessel window must be "
			<< "finite and 0 or more, but it is " << shape << ".";
		throw std::invalid_argument(oss.str());
	}

	return gridding_window(gridding_profile::kaiser_bessel, half_width, shape);
}

gridding_profile gridding_window::get_profile() const noexcept
{
	return m_profile;
}

double gridding_window::get_half_width() const noexcept
{
	return m_half_width;
}

double gridding_window::get_shape() const noexcept
{
	return m_shape;
}

gridding_window::gridding_window(
	gridding_profile profile,
	double half_width,
	double shape
) noexcept
	: m_profile(profile)
	, m_half_width(half_width)
	, m_shape(shape)
{
}

} // namespace em
} // namespace rexlib
