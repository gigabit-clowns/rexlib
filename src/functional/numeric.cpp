// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/functional/numeric.hpp>

#include <xmipp4/core/dispatch/execute.hpp>
#include <xmipp4/ops/numeric/copysign_operation.hpp>
#include <xmipp4/ops/numeric/is_finite_operation.hpp>
#include <xmipp4/ops/numeric/is_inf_operation.hpp>
#include <xmipp4/ops/numeric/is_nan_operation.hpp>
#include <xmipp4/ops/numeric/sign_bit_operation.hpp>

namespace rex
{

array is_nan(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::is_nan_operation(), x, context, out);
}

array is_inf(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::is_inf_operation(), x, context, out);
}

array is_finite(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::is_finite_operation(), x, context, out);
}

array sign_bit(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::sign_bit_operation(), x, context, out);
}

array copysign(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::copysign_operation(), x, y, context, out);
}

} // namespace rex
