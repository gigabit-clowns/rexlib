// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/functional/preprocessing.hpp>

#include <functional/array_axes.hpp>

#include <rexlib/core/dispatch/execute.hpp>
#include <rexlib/em/ops/preprocessing/standardize_background_operation.hpp>

namespace rexlib
{
namespace em
{

array standardize_background(
	const_array_ref value,
	const_array_ref seed,
	span<const std::ptrdiff_t> axes,
	double radius,
	double lower_bound,
	double upper_bound,
	const execution_context &context,
	array *out
)
{
	return execute_binary(
		standardize_background_operation(
			resolve_axes(value, axes),
			radius,
			lower_bound,
			upper_bound
		),
		value,
		seed,
		context,
		out
	);
}

array standardize_background_2d(
	const_array_ref value,
	const_array_ref seed,
	double radius,
	double lower_bound,
	double upper_bound,
	const execution_context &context,
	array *out
)
{
	return execute_binary(
		standardize_background_operation(
			last_axes(value, 2),
			radius,
			lower_bound,
			upper_bound
		),
		value,
		seed,
		context,
		out
	);
}

array standardize_background_3d(
	const_array_ref value,
	const_array_ref seed,
	double radius,
	double lower_bound,
	double upper_bound,
	const execution_context &context,
	array *out
)
{
	return execute_binary(
		standardize_background_operation(
			last_axes(value, 3),
			radius,
			lower_bound,
			upper_bound
		),
		value,
		seed,
		context,
		out
	);
}

} // namespace em
} // namespace rexlib
