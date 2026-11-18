// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/skeletons/fourier/fourier_layout_plan.hpp>

#include <rexlib/ops/fourier/fourier_normalization.hpp>

#include <complex>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Transform a real operand into the half spectrum a real signal needs.
 *
 * @tparam Fft The Fourier transform algorithm, such as @ref pocketfft_fft.
 * It provides a static real_to_complex member with the signature
 * pocketfft_fft has.
 */
template <typename Fft>
class real_to_complex_fourier_transform
{
public:
	/**
	 * @brief Construct the transform of a scaling convention.
	 *
	 * @param normalization Which of the transform pair carries the scaling.
	 */
	explicit real_to_complex_fourier_transform(
		ops::fourier_normalization normalization
	) noexcept;

	/**
	 * @brief Get the domain each operand lives in.
	 *
	 * @return fourier_transform_kind Always
	 * fourier_transform_kind::real_to_complex.
	 */
	static fourier_transform_kind get_kind() noexcept;

	/**
	 * @brief Transform a real operand forward.
	 *
	 * @tparam T The component type.
	 * @param plan The planned transform.
	 * @param output The half spectrum, displaced by its offset.
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

} // namespace cpu
} // namespace rexlib

#include "real_to_complex_fourier_transform.inl"
