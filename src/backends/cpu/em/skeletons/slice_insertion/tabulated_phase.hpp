// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <complex>
#include <cstddef>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The phase a voxel of a volume carries, read from a table per axis.
 *
 * exp(-2 pi i g . C / N) with C = N / 2 rounded down is the product of one
 * factor per axis, tabulated by stored index.
 *
 * @tparam T The real type of the factors.
 */
template <typename T>
class tabulated_phase
{
public:
	/**
	 * @brief Tabulate the phases of a volume.
	 *
	 * @param extent The extent N of the volume along each axis.
	 */
	explicit tabulated_phase(std::size_t extent);

	/**
	 * @brief Apply the phase of a voxel to a value.
	 *
	 * @param value The value.
	 * @param x The frequency along x. In [0, N / 2].
	 * @param stored_y The stored index along y.
	 * @param stored_z The stored index along z.
	 * @return std::complex<T> The value times the phase.
	 */
	std::complex<T> apply(
		std::complex<T> value,
		std::ptrdiff_t x,
		std::ptrdiff_t stored_y,
		std::ptrdiff_t stored_z
	) const noexcept;

private:
	std::vector<std::complex<T>> m_x;
	std::vector<std::complex<T>> m_full;
};

} // namespace cpu
} // namespace rexlib

#include "tabulated_phase.inl"
