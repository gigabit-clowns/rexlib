// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/kernels/pocketfft_fourier_transform.hpp>

#include <backends/cpu/config.hpp>

#include <rex/core/platform/compiler.h>

#include <complex>

// pocketfft parks its workers in a function local static whose destructor
// joins them. Built with mingw-w64 that hangs, and the threading is off there
// for that reason alone: upstream reports it against g++/mingw-w64 and states
// that MSVC is unaffected, without a diagnosis of the cause, and the fix
// attempted since has not worked.
//
//   https://github.com/mreineck/pocketfft/issues/1
//   https://github.com/scipy/scipy/issues/16352
//
// SciPy meets the same thing and answers it the same way: conda-forge and
// MSYS2 ship it with this define, while the MSVC wheels keep their threading.
// Every toolchain but mingw-w64 is left threaded here for the same reason,
// the transform being the one part of this backend that is not spread over
// cpu::thread_pool.
//
// This is the only translation unit that includes pocketfft, so defining it
// here cannot make one build of the header disagree with another.
#ifdef REXLIB_MINGW
	#define POCKETFFT_NO_MULTITHREADING
#endif
#define POCKETFFT_CACHE_SIZE REXLIB_POCKETFFT_CACHE_SIZE
#include <pocketfft_hdronly.h>

namespace rex
{
namespace cpu
{

namespace
{

bool is_forward(ops::fourier_direction direction) noexcept
{
	return direction == ops::fourier_direction::forward;
}

/**
 * @brief The factor a transform scales its result by.
 *
 * Derived by the operations rather than here, so that every backend
 * implementing them arrives at the same number. Computed in double and
 * rounded once into the type the transform runs in.
 */
template <typename T>
T get_scale(
	const fourier_layout_plan &plan,
	ops::fourier_direction direction,
	ops::fourier_normalization normalization
) noexcept
{
	return static_cast<T>(
		ops::get_fourier_scale(
			normalization,
			direction,
			plan.get_sample_count()
		)
	);
}

} // anonymous namespace

template <typename T>
void run_complex_to_complex_transform(
	const fourier_layout_plan &plan,
	ops::fourier_direction direction,
	ops::fourier_normalization normalization,
	std::complex<T> *output,
	const std::complex<T> *input,
	std::size_t nthreads
)
{
	pocketfft::c2c(
		plan.get_shape(),
		plan.get_input_strides(),
		plan.get_output_strides(),
		plan.get_axes(),
		is_forward(direction),
		input,
		output,
		get_scale<T>(plan, direction, normalization),
		nthreads
	);
}

template <typename T>
void run_in_place_complex_transform(
	const fourier_layout_plan &plan,
	ops::fourier_direction direction,
	ops::fourier_normalization normalization,
	std::complex<T> *data,
	std::size_t nthreads
)
{
	pocketfft::c2c(
		plan.get_shape(),
		plan.get_output_strides(),
		plan.get_output_strides(),
		plan.get_axes(),
		is_forward(direction),
		data,
		data,
		get_scale<T>(plan, direction, normalization),
		nthreads
	);
}

template <typename T>
void run_real_to_complex_transform(
	const fourier_layout_plan &plan,
	ops::fourier_direction direction,
	ops::fourier_normalization normalization,
	std::complex<T> *output,
	const T *input,
	std::size_t nthreads
)
{
	pocketfft::r2c(
		plan.get_shape(),
		plan.get_input_strides(),
		plan.get_output_strides(),
		plan.get_axes(),
		is_forward(direction),
		input,
		output,
		get_scale<T>(plan, direction, normalization),
		nthreads
	);
}

template <typename T>
void run_complex_to_real_transform(
	const fourier_layout_plan &plan,
	ops::fourier_direction direction,
	ops::fourier_normalization normalization,
	T *output,
	const std::complex<T> *input,
	std::size_t nthreads
)
{
	pocketfft::c2r(
		plan.get_shape(),
		plan.get_input_strides(),
		plan.get_output_strides(),
		plan.get_axes(),
		is_forward(direction),
		input,
		output,
		get_scale<T>(plan, direction, normalization),
		nthreads
	);
}

// pocketfft is a header the size of a small library, and the transform is the
// one thing in this backend that does not have to be inlined into a caller:
// it is entered once per program run, not once per element. Instantiating it
// here is what keeps that header out of every builder that registers one.
template void run_complex_to_complex_transform<float32_t>(
	const fourier_layout_plan&,
	ops::fourier_direction,
	ops::fourier_normalization,
	std::complex<float32_t>*,
	const std::complex<float32_t>*,
	std::size_t
);
template void run_complex_to_complex_transform<float64_t>(
	const fourier_layout_plan&,
	ops::fourier_direction,
	ops::fourier_normalization,
	std::complex<float64_t>*,
	const std::complex<float64_t>*,
	std::size_t
);

template void run_in_place_complex_transform<float32_t>(
	const fourier_layout_plan&,
	ops::fourier_direction,
	ops::fourier_normalization,
	std::complex<float32_t>*,
	std::size_t
);
template void run_in_place_complex_transform<float64_t>(
	const fourier_layout_plan&,
	ops::fourier_direction,
	ops::fourier_normalization,
	std::complex<float64_t>*,
	std::size_t
);

template void run_real_to_complex_transform<float32_t>(
	const fourier_layout_plan&,
	ops::fourier_direction,
	ops::fourier_normalization,
	std::complex<float32_t>*,
	const float32_t*,
	std::size_t
);
template void run_real_to_complex_transform<float64_t>(
	const fourier_layout_plan&,
	ops::fourier_direction,
	ops::fourier_normalization,
	std::complex<float64_t>*,
	const float64_t*,
	std::size_t
);

template void run_complex_to_real_transform<float32_t>(
	const fourier_layout_plan&,
	ops::fourier_direction,
	ops::fourier_normalization,
	float32_t*,
	const std::complex<float32_t>*,
	std::size_t
);
template void run_complex_to_real_transform<float64_t>(
	const fourier_layout_plan&,
	ops::fourier_direction,
	ops::fourier_normalization,
	float64_t*,
	const std::complex<float64_t>*,
	std::size_t
);

} // namespace cpu
} // namespace rex
