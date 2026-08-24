// SPDX-License-Identifier: GPL-3.0-only

#include <rex/functional/bitwise.hpp>

#include <rex/core/dispatch/execute.hpp>
#include <rex/ops/bitwise/bitwise_and_operation.hpp>
#include <rex/ops/bitwise/bitwise_not_operation.hpp>
#include <rex/ops/bitwise/bitwise_or_operation.hpp>
#include <rex/ops/bitwise/bitwise_xor_operation.hpp>
#include <rex/ops/bitwise/left_shift_operation.hpp>
#include <rex/ops/bitwise/right_shift_operation.hpp>

namespace rexlib
{

array bitwise_and(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::bitwise_and_operation(), x, y, context, out);
}

array bitwise_or(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::bitwise_or_operation(), x, y, context, out);
}

array bitwise_xor(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::bitwise_xor_operation(), x, y, context, out);
}

array bitwise_not(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::bitwise_not_operation(), x, context, out);
}

array left_shift(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::left_shift_operation(), x, y, context, out);
}

array right_shift(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::right_shift_operation(), x, y, context, out);
}

} // namespace rexlib
