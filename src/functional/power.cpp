// SPDX-License-Identifier: GPL-3.0-only

#include <rex/functional/power.hpp>

#include <rex/core/dispatch/execute.hpp>
#include <rex/ops/power/cbrt_operation.hpp>
#include <rex/ops/power/exp_operation.hpp>
#include <rex/ops/power/exp2_operation.hpp>
#include <rex/ops/power/expm1_operation.hpp>
#include <rex/ops/power/log_operation.hpp>
#include <rex/ops/power/log10_operation.hpp>
#include <rex/ops/power/log1p_operation.hpp>
#include <rex/ops/power/log2_operation.hpp>
#include <rex/ops/power/power_operation.hpp>
#include <rex/ops/power/reciprocal_operation.hpp>
#include <rex/ops/power/sqrt_operation.hpp>
#include <rex/ops/power/square_operation.hpp>

namespace rexlib
{

array power(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::power_operation(), x, y, context, out);
}

array square(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::square_operation(), x, context, out);
}

array reciprocal(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::reciprocal_operation(), x, context, out);
}

array sqrt(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::sqrt_operation(), x, context, out);
}

array cbrt(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::cbrt_operation(), x, context, out);
}

array exp(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::exp_operation(), x, context, out);
}

array exp2(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::exp2_operation(), x, context, out);
}

array expm1(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::expm1_operation(), x, context, out);
}

array log(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::log_operation(), x, context, out);
}

array log2(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::log2_operation(), x, context, out);
}

array log10(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::log10_operation(), x, context, out);
}

array log1p(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::log1p_operation(), x, context, out);
}

} // namespace rexlib
