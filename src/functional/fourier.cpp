// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/functional/fourier.hpp>

#include "array_axes.hpp"

#include <rexlib/core/dispatch/execute.hpp>
#include <rexlib/ops/parameters/axis_list.hpp>
#include <rexlib/ops/fourier/fft_operation.hpp>
#include <rexlib/ops/fourier/fftshift_operation.hpp>
#include <rexlib/ops/fourier/ifft_operation.hpp>
#include <rexlib/ops/fourier/ifftshift_operation.hpp>
#include <rexlib/ops/fourier/irfft_operation.hpp>
#include <rexlib/ops/fourier/rfft_operation.hpp>

namespace rexlib
{

array fft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::fft_operation(resolve_axes(x, axes), normalization),
		x,
		context,
		out
	);
}

array fft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	const execution_context &context,
	array *out
)
{
	return fft(
		x,
		axes,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array fft(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::fft_operation(last_axes(x, 1), normalization),
		x,
		context,
		out
	);
}

array fft(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return fft(
		x,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array fft2(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::fft_operation(last_axes(x, 2), normalization),
		x,
		context,
		out
	);
}

array fft2(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return fft2(
		x,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array fft3(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::fft_operation(last_axes(x, 3), normalization),
		x,
		context,
		out
	);
}

array fft3(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return fft3(
		x,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array ifft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::ifft_operation(resolve_axes(x, axes), normalization),
		x,
		context,
		out
	);
}

array ifft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	const execution_context &context,
	array *out
)
{
	return ifft(
		x,
		axes,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array ifft(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::ifft_operation(last_axes(x, 1), normalization),
		x,
		context,
		out
	);
}

array ifft(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return ifft(
		x,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array ifft2(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::ifft_operation(last_axes(x, 2), normalization),
		x,
		context,
		out
	);
}

array ifft2(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return ifft2(
		x,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array ifft3(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::ifft_operation(last_axes(x, 3), normalization),
		x,
		context,
		out
	);
}

array ifft3(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return ifft3(
		x,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array rfft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::rfft_operation(resolve_axes(x, axes), normalization),
		x,
		context,
		out
	);
}

array rfft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	const execution_context &context,
	array *out
)
{
	return rfft(
		x,
		axes,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array rfft(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::rfft_operation(last_axes(x, 1), normalization),
		x,
		context,
		out
	);
}

array rfft(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return rfft(
		x,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array rfft2(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::rfft_operation(last_axes(x, 2), normalization),
		x,
		context,
		out
	);
}

array rfft2(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return rfft2(
		x,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array rfft3(
	const_array_ref x,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::rfft_operation(last_axes(x, 3), normalization),
		x,
		context,
		out
	);
}

array rfft3(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return rfft3(
		x,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array irfft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	std::size_t extent,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::irfft_operation(
			resolve_axes(x, axes),
			extent,
			normalization
		),
		x,
		context,
		out
	);
}

array irfft(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	std::size_t extent,
	const execution_context &context,
	array *out
)
{
	return irfft(
		x,
		axes,
		extent,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array irfft(
	const_array_ref x,
	std::size_t extent,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::irfft_operation(
			last_axes(x, 1),
			extent,
			normalization
		),
		x,
		context,
		out
	);
}

array irfft(
	const_array_ref x,
	std::size_t extent,
	const execution_context &context,
	array *out
)
{
	return irfft(
		x,
		extent,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array irfft2(
	const_array_ref x,
	std::size_t extent,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::irfft_operation(
			last_axes(x, 2),
			extent,
			normalization
		),
		x,
		context,
		out
	);
}

array irfft2(
	const_array_ref x,
	std::size_t extent,
	const execution_context &context,
	array *out
)
{
	return irfft2(
		x,
		extent,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array irfft3(
	const_array_ref x,
	std::size_t extent,
	ops::fourier_normalization normalization,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::irfft_operation(
			last_axes(x, 3),
			extent,
			normalization
		),
		x,
		context,
		out
	);
}

array irfft3(
	const_array_ref x,
	std::size_t extent,
	const execution_context &context,
	array *out
)
{
	return irfft3(
		x,
		extent,
		ops::fourier_normalization::backward,
		context,
		out
	);
}

array fftshift(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::fftshift_operation(resolve_axes(x, axes)),
		x,
		context,
		out
	);
}

array fftshift(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::fftshift_operation(every_axis(x)),
		x,
		context,
		out
	);
}

array ifftshift(
	const_array_ref x,
	span<const std::ptrdiff_t> axes,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::ifftshift_operation(resolve_axes(x, axes)),
		x,
		context,
		out
	);
}

array ifftshift(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(
		ops::ifftshift_operation(every_axis(x)),
		x,
		context,
		out
	);
}

} // namespace rexlib
