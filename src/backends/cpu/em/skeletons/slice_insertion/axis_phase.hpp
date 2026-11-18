// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <complex>
#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Fill the phase factors of a shift along one axis of a spectrum.
 *
 * The entry j receives exp(2 pi i f_j shift / extent), where f_j is the
 * frequency of the index j: j up to extent / 2, rounded down, and
 * j - extent beyond. The first count entries are filled, so a half axis
 * fills extent / 2 + 1 of them and a full axis fills extent.
 *
 * @tparam T The real type of the factors.
 * @param out Receives the factors.
 * @param count The number of factors.
 * @param extent The number of samples of the axis.
 * @param shift The shift, in samples.
 */
template <typename T>
void fill_axis_phase(
	std::complex<T> *out,
	std::size_t count,
	std::size_t extent,
	double shift
) noexcept;

} // namespace cpu
} // namespace rexlib

#include "axis_phase.inl"
