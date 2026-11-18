// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <complex>
#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The phase a voxel of a volume of even extent carries: a sign.
 *
 * exp(-2 pi i g . C / N) with C = N / 2 is (-1) to the sum of the
 * coordinates of g, whichever way round the coordinates are stored.
 */
class alternating_sign
{
public:
	/**
	 * @brief Apply the phase of a voxel to a value.
	 *
	 * @tparam T The real type of the value.
	 * @param value The value.
	 * @param x The frequency along x.
	 * @param stored_y The stored index along y.
	 * @param stored_z The stored index along z.
	 * @return std::complex<T> The value, negated for an odd sum.
	 */
	template <typename T>
	std::complex<T> apply(
		std::complex<T> value,
		std::ptrdiff_t x,
		std::ptrdiff_t stored_y,
		std::ptrdiff_t stored_z
	) const noexcept;
};

} // namespace cpu
} // namespace rexlib

#include "alternating_sign.inl"
