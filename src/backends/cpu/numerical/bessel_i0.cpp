// SPDX-License-Identifier: GPL-3.0-only

#include "bessel_i0.hpp"

#include <limits>

namespace rexlib
{
namespace cpu
{

double bessel_i0(double x) noexcept
{
	const auto quarter_square = 0.25 * x * x;
	double sum = 1.0;
	double term = 1.0;
	for (double k = 1.0; ; k += 1.0)
	{
		term *= quarter_square / (k * k);
		sum += term;
		if (term <= std::numeric_limits<double>::epsilon() * sum)
		{
			return sum;
		}
	}
}

} // namespace cpu
} // namespace rexlib
