// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/pocketfft/fft/is_fourier_transformable.hpp>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Support predicate narrowing a Fourier builder to what pocketfft has.
 *
 * Every Fourier typing rule pivots on the single operand whose type is free,
 * so one pivot is all this ever sees.
 *
 * @see is_fourier_transformable
 */
template <typename... Pivots>
struct fourier_transform_support;

template <typename Pivot>
struct fourier_transform_support<Pivot>
	: is_fourier_transformable<Pivot>
{
};

} // namespace cpu
} // namespace rexlib
