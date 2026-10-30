// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/preprocessing/standardize_background_operation.hpp>

#include <sstream>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{

namespace
{

double check_radius(double radius)
{
	if (!(radius >= 0.0))
	{
		std::ostringstream oss;
		oss << "standardize_background: the radius must be 0 or more, but it "
			<< "is " << radius << ".";
		throw std::invalid_argument(oss.str());
	}
	return radius;
}

void check_bounds(double lower_bound, double upper_bound)
{
	if (!(lower_bound < 0.0 && upper_bound > 0.0))
	{
		std::ostringstream oss;
		oss << "standardize_background: the bounds must lie on either side "
			<< "of 0, but they are " << lower_bound << " and " << upper_bound
			<< ".";
		throw std::invalid_argument(oss.str());
	}
}

} // anonymous namespace

standardize_background_operation::standardize_background_operation(
	ops::axis_list axes,
	double radius,
	double lower_bound,
	double upper_bound
)
	: parametric_operation(std::move(axes))
	, m_radius(check_radius(radius))
	, m_lower_bound(lower_bound)
	, m_upper_bound(upper_bound)
{
	check_bounds(m_lower_bound, m_upper_bound);
}

double standardize_background_operation::get_radius() const noexcept
{
	return m_radius;
}

double standardize_background_operation::get_lower_bound() const noexcept
{
	return m_lower_bound;
}

double standardize_background_operation::get_upper_bound() const noexcept
{
	return m_upper_bound;
}

} // namespace em
} // namespace rexlib
