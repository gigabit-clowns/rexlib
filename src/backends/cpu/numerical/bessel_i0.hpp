// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{
namespace cpu
{

/**
 * @brief Compute the modified Bessel function of the first kind and
 * order 0.
 *
 * The power series is summed until a term falls below the rounding of the
 * sum, which takes some 30 terms at x = 15.
 *
 * @param x The argument. Must be finite.
 * @return double I0(x).
 */
double bessel_i0(double x) noexcept;

} // namespace cpu
} // namespace rexlib
