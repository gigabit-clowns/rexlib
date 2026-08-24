// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/functional/selection.hpp>

#include <rexlib/core/dispatch/execute.hpp>
#include <rexlib/ops/selection/clip_operation.hpp>
#include <rexlib/ops/selection/maximum_operation.hpp>
#include <rexlib/ops/selection/minimum_operation.hpp>
#include <rexlib/ops/selection/where_operation.hpp>

namespace rexlib
{

array where(
	const_array_ref condition,
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_ternary(ops::where_operation(), condition, x, y, context, out);
}

array clip(
	const_array_ref x,
	const_array_ref lower,
	const_array_ref upper,
	const execution_context &context,
	array *out
)
{
	return execute_ternary(ops::clip_operation(), x, lower, upper, context, out);
}

array maximum(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::maximum_operation(), x, y, context, out);
}

array minimum(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::minimum_operation(), x, y, context, out);
}

} // namespace rexlib
