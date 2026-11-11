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
 * @brief Transform a half spectrum back into the real signal it came from.
 *
 * @tparam Fft The Fourier transform algorithm, such as @ref pocketfft_fft.
 * It provides a static complex_to_real member with the signature
 * pocketfft_fft has.
 */
template <typename Fft>
class complex_to_real_fourier_transform
{
public:
	/**
	 * @brief Construct the transform of a scaling convention.
	 *
	 * @param normalization Which of the transform pair carries the scaling.
	 */
	explicit complex_to_real_fourier_transform(
		ops::fourier_normalization normalization
	) noexcept;

	/**
	 * @brief Get the domain each operand lives in.
	 *
	 * @return fourier_transform_kind Always
	 * fourier_transform_kind::complex_to_real.
	 */
	static fourier_transform_kind get_kind() noexcept;

	/**
	 * @brief Transform a half spectrum back.
	 *
	 * @tparam T The component type.
	 * @param plan The planned transform.
	 * @param output The real operand, displaced by its offset.
	 * @param input The half spectrum, displaced by its offset.
	 * @param schedule The threads to spread the transform over.
	 */
	template <typename T>
	void operator()(
		const fourier_layout_plan &plan,
		T *output,
		const std::complex<T> *input,
		const loop_schedule &schedule
	) const;

private:
	ops::fourier_normalization m_normalization;
};

} // namespace cpu
} // namespace rexlib

#include "complex_to_real_fourier_transform.inl"
