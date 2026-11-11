// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/plans/fourier_layout_plan.hpp>

#include <rexlib/ops/fourier/fourier_normalization.hpp>

#include <complex>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Transform between two complex operands of the same extents.
 *
 * Serves both the forward transform and its inverse, which differ in the
 * direction alone, and admits the real operand the forward transform also
 * takes. A real operand is widened into the output and transformed there. A
 * transform along no axis is the identity, and copies the operand across.
 *
 * @tparam Fft The Fourier transform algorithm, such as @ref pocketfft_fft.
 * It provides static complex_to_complex and in_place_complex members with
 * the signatures pocketfft_fft has, and is never asked to transform along no
 * axis.
 * @tparam Direction Which way round to transform.
 */
template <typename Fft, ops::fourier_direction Direction>
class complex_fourier_transform
{
public:
	/**
	 * @brief Construct the transform of a scaling convention.
	 *
	 * @param normalization Which of the transform pair carries the scaling.
	 */
	explicit complex_fourier_transform(
		ops::fourier_normalization normalization
	) noexcept;

	/**
	 * @brief Get the domain each operand lives in.
	 *
	 * @return fourier_transform_kind Always
	 * fourier_transform_kind::complex_to_complex.
	 */
	static fourier_transform_kind get_kind() noexcept;

	/**
	 * @brief Transform a complex operand.
	 *
	 * @tparam T The component type.
	 * @param plan The planned transform.
	 * @param output The transformed operand, displaced by its offset.
	 * @param input The operand to transform, displaced by its offset.
	 * @param schedule The threads to spread the transform over.
	 */
	template <typename T>
	void operator()(
		const fourier_layout_plan &plan,
		std::complex<T> *output,
		const std::complex<T> *input,
		const loop_schedule &schedule
	) const;

	/**
	 * @brief Transform a real operand.
	 *
	 * @tparam T The component type.
	 * @param plan The planned transform.
	 * @param output The full spectrum, displaced by its offset.
	 * @param input The operand to transform, displaced by its offset.
	 * @param schedule The threads to spread the transform over.
	 */
	template <typename T>
	void operator()(
		const fourier_layout_plan &plan,
		std::complex<T> *output,
		const T *input,
		const loop_schedule &schedule
	) const;

private:
	ops::fourier_normalization m_normalization;
};

/**
 * @brief The forward transform of a complex signal.
 *
 * @tparam Fft The Fourier transform algorithm.
 */
template <typename Fft>
using forward_complex_fourier_transform =
	complex_fourier_transform<Fft, ops::fourier_direction::forward>;

/**
 * @brief The inverse transform of a complex spectrum.
 *
 * @tparam Fft The Fourier transform algorithm.
 */
template <typename Fft>
using inverse_complex_fourier_transform =
	complex_fourier_transform<Fft, ops::fourier_direction::inverse>;

} // namespace cpu
} // namespace rexlib

#include "complex_fourier_transform.inl"
