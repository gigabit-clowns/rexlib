// SPDX-License-Identifier: GPL-3.0-only

#include <rex/functional/logical.hpp>

#include <rex/core/dispatch/execute.hpp>
#include <rex/ops/logical/logical_and_operation.hpp>
#include <rex/ops/logical/logical_not_operation.hpp>
#include <rex/ops/logical/logical_or_operation.hpp>
#include <rex/ops/logical/logical_xor_operation.hpp>

namespace rexlib
{

array logical_and(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::logical_and_operation(), x, y, context, out);
}

array logical_or(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::logical_or_operation(), x, y, context, out);
}

array logical_xor(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::logical_xor_operation(), x, y, context, out);
}

array logical_not(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::logical_not_operation(), x, context, out);
}

} // namespace rexlib
