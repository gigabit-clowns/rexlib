// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/fixed_width_float.hpp>

#include <complex>
#include <type_traits>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Whether pocketfft can transform elements of a storage type.
 *
 * pocketfft computes in single or double precision. Half precision has
 * neither: it is a storage type that this backend widens to compute with, and
 * a transform cannot be widened element by element the way an elementwise
 * kernel can, so it would have to be staged through a whole array of its own.
 * Nothing asks for that yet, and a spectrum of any size leaves half
 * precision's range long before it stops being accurate, so the backend
 * reports it unsupported instead of pretending.
 *
 * @tparam T The storage element type.
 */
template <typename T>
struct is_fourier_transformable
	: std::false_type
{
};

template <>
struct is_fourier_transformable<float32_t>
	: std::true_type
{
};

template <>
struct is_fourier_transformable<float64_t>
	: std::true_type
{
};

template <typename T>
struct is_fourier_transformable<std::complex<T>>
	: is_fourier_transformable<T>
{
};

} // namespace cpu
} // namespace rexlib
