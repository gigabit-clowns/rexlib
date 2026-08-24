// SPDX-License-Identifier: GPL-3.0-only

#include <rex/functional/rounding.hpp>

#include <rex/core/dispatch/execute.hpp>
#include <rex/ops/rounding/ceil_operation.hpp>
#include <rex/ops/rounding/floor_operation.hpp>
#include <rex/ops/rounding/round_operation.hpp>
#include <rex/ops/rounding/trunc_operation.hpp>

namespace rexlib
{

array floor(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::floor_operation(), x, context, out);
}

array ceil(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::ceil_operation(), x, context, out);
}

array trunc(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::trunc_operation(), x, context, out);
}

array round(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::round_operation(), x, context, out);
}

} // namespace rexlib
