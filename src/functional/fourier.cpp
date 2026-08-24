// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/functional/fourier.hpp>

#include <rexlib/core/dispatch/execute.hpp>
#include <rexlib/core/layout/index.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/ops/policies/axis_list.hpp>
#include <rexlib/ops/fourier/fft_operation.hpp>
#include <rexlib/ops/fourier/fftshift_operation.hpp>
#include <rexlib/ops/fourier/ifft_operation.hpp>
#include <rexlib/ops/fourier/ifftshift_operation.hpp>
#include <rexlib/ops/fourier/irfft_operation.hpp>
#include <rexlib/ops/fourier/rfft_operation.hpp>

#include <numeric>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace rexlib
{

namespace
{

std::size_t get_rank(const const_array_ref &x)
{
	std::vector<std::size_t> extents;
	x.get_descriptor().get_layout().get_extents(extents);
	return extents.size();
}

/**
 * @brief Turn the axes a caller gave into the ones an operation holds.
 *
 * Negative axes are referred from the end. An operation is a rigid
 * description of the work to be done, so this happens here, where the
 * array and therefore its rank are at hand.
 */
ops::axis_list resolve_axes(
	const const_array_ref &x,
	span<const std::ptrdiff_t> axes
)
{
	const auto rank = get_rank(x);

	ops::axis_list result;
	result.reserve(axes.size());
	for (const auto axis : axes)
	{
		result.push_back(sanitize_index(axis, rank));
	}

	return result;
}

/// Name every axis of an array.
ops::axis_list every_axis(const const_array_ref &x)
{
	ops::axis_list result(get_rank(x));
	std::iota(result.begin(), result.end(), std::size_t(0));
	return result;
}

/**
 * @brief Name the trailing @p count axes of an array.
 *
 * What the numbered spellings transform along. An image lives in the last
 * two axes of a stack of them, and a volume in the last three, so this is
 * the shape the convenience forms exist to express.
 */
ops::axis_list last_axes(const const_array_ref &x, std::size_t count)
{
	const auto rank = get_rank(x);
	if (rank < count)
	{
		std::ostringstream oss;
		oss << "A " << count << " dimensional transform needs at least "
			<< count << " axes, but the operand has " << rank << ".";
		throw std::invalid_argument(oss.str());
	}

	ops::axis_list result(count);
	std::iota(result.begin(), result.end(), rank - count);
	return result;
}

} // anonymous namespace

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
	// An operation carries the parity, that being the whole of what the
	// restoration needs. Naming an extent instead is a convenience, and
	// this is where the two meet.
	return execute_unary(
		ops::irfft_operation(
			resolve_axes(x, axes),
			ops::get_signal_parity(extent),
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
			ops::get_signal_parity(extent),
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
			ops::get_signal_parity(extent),
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
			ops::get_signal_parity(extent),
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
