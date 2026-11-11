// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/skeletons/fourier/fourier_layout_plan.hpp>

#include <rexlib/core/numerical/fixed_width_float.hpp>
#include <rexlib/ops/fourier/fourier_normalization.hpp>

#include <complex>
#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Fourier transforms computed by pocketfft.
 *
 * Each transform reads the shape, strides and axes of a fourier_layout_plan
 * and is handed the operand pointers already displaced by their offsets. The
 * plan names at least one axis. pocketfft computes in single and double
 * precision only, see @ref fourier_transform_support.
 *
 * pocketfft threads a transform from a pool of its own, so each transform
 * takes a thread count rather than a cpu::thread_pool. The count is ignored
 * where that threading is off, see the definition.
 */
class pocketfft_fft final
{
public:
	/**
	 * @brief Transform a complex operand into another one.
	 *
	 * @tparam T The component type. Instantiated for @ref float32_t and
	 * @ref float64_t.
	 * @param plan The planned transform.
	 * @param direction Which way round to transform.
	 * @param normalization Which of the transform pair carries the scaling.
	 * @param output The transformed operand.
	 * @param input The operand to transform.
	 * @param thread_count How many threads pocketfft may use.
	 */
	template <typename T>
	static void complex_to_complex(
		const fourier_layout_plan &plan,
		ops::fourier_direction direction,
		ops::fourier_normalization normalization,
		std::complex<T> *output,
		const std::complex<T> *input,
		std::size_t thread_count
	);

	/**
	 * @brief Transform a complex operand where it lies.
	 *
	 * The operand is described by the output strides on both sides.
	 *
	 * @tparam T The component type. Instantiated for @ref float32_t and
	 * @ref float64_t.
	 * @param plan The planned transform.
	 * @param direction Which way round to transform.
	 * @param normalization Which of the transform pair carries the scaling.
	 * @param data The operand, at the output offset.
	 * @param thread_count How many threads pocketfft may use.
	 */
	template <typename T>
	static void in_place_complex(
		const fourier_layout_plan &plan,
		ops::fourier_direction direction,
		ops::fourier_normalization normalization,
		std::complex<T> *data,
		std::size_t thread_count
	);

	/**
	 * @brief Transform a real operand into a half spectrum.
	 *
	 * @tparam T The component type. Instantiated for @ref float32_t and
	 * @ref float64_t.
	 * @param plan The planned transform.
	 * @param direction Which way round to transform.
	 * @param normalization Which of the transform pair carries the scaling.
	 * @param output The half spectrum.
	 * @param input The real operand.
	 * @param thread_count How many threads pocketfft may use.
	 */
	template <typename T>
	static void real_to_complex(
		const fourier_layout_plan &plan,
		ops::fourier_direction direction,
		ops::fourier_normalization normalization,
		std::complex<T> *output,
		const T *input,
		std::size_t thread_count
	);

	/**
	 * @brief Transform a half spectrum back into a real operand.
	 *
	 * @tparam T The component type. Instantiated for @ref float32_t and
	 * @ref float64_t.
	 * @param plan The planned transform.
	 * @param direction Which way round to transform.
	 * @param normalization Which of the transform pair carries the scaling.
	 * @param output The real operand.
	 * @param input The half spectrum.
	 * @param thread_count How many threads pocketfft may use.
	 */
	template <typename T>
	static void complex_to_real(
		const fourier_layout_plan &plan,
		ops::fourier_direction direction,
		ops::fourier_normalization normalization,
		T *output,
		const std::complex<T> *input,
		std::size_t thread_count
	);
};

} // namespace cpu
} // namespace rexlib
