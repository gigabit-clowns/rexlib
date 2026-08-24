// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/functional/trigonometric.hpp>

#include <rexlib/core/dispatch/execute.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/ops/trigonometric/acos_operation.hpp>
#include <rexlib/ops/trigonometric/acosh_operation.hpp>
#include <rexlib/ops/trigonometric/asin_operation.hpp>
#include <rexlib/ops/trigonometric/asinh_operation.hpp>
#include <rexlib/ops/trigonometric/atan_operation.hpp>
#include <rexlib/ops/trigonometric/atan2_operation.hpp>
#include <rexlib/ops/trigonometric/atanh_operation.hpp>
#include <rexlib/ops/trigonometric/cos_operation.hpp>
#include <rexlib/ops/trigonometric/cosh_operation.hpp>
#include <rexlib/ops/trigonometric/degrees_operation.hpp>
#include <rexlib/ops/trigonometric/hypot_operation.hpp>
#include <rexlib/ops/trigonometric/radians_operation.hpp>
#include <rexlib/ops/trigonometric/sin_operation.hpp>
#include <rexlib/ops/trigonometric/sincos_operation.hpp>
#include <rexlib/ops/trigonometric/sinh_operation.hpp>
#include <rexlib/ops/trigonometric/tan_operation.hpp>
#include <rexlib/ops/trigonometric/tanh_operation.hpp>

#include <array>
#include <utility>

namespace rexlib
{

array sin(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::sin_operation(), x, context, out);
}

array cos(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::cos_operation(), x, context, out);
}

array tan(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::tan_operation(), x, context, out);
}

array asin(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::asin_operation(), x, context, out);
}

array acos(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::acos_operation(), x, context, out);
}

array atan(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::atan_operation(), x, context, out);
}

array sinh(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::sinh_operation(), x, context, out);
}

array cosh(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::cosh_operation(), x, context, out);
}

array tanh(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::tanh_operation(), x, context, out);
}

array asinh(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::asinh_operation(), x, context, out);
}

array acosh(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::acosh_operation(), x, context, out);
}

array atanh(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::atanh_operation(), x, context, out);
}

array atan2(
	const_array_ref y,
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::atan2_operation(), y, x, context, out);
}

array hypot(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::hypot_operation(), x, y, context, out);
}

array degrees(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::degrees_operation(), x, context, out);
}

array radians(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::radians_operation(), x, context, out);
}

std::pair<array, array> sincos(
	const_array_ref x,
	const execution_context &context,
	array *sine,
	array *cosine
)
{
	// Seed the outputs from whichever the caller supplied, so that their
	// storage is reused. The dispatcher writes back into this array, which
	// is why the results come out of it rather than out of a return value.
	std::array<array, 2> outputs;
	if (sine)
	{
		outputs[0] = sine->share();
	}
	if (cosine)
	{
		outputs[1] = cosine->share();
	}

	execute_unary(
		ops::sincos_operation(),
		make_span(outputs),
		x,
		context
	);

	return std::make_pair(
		std::move(outputs[0]),
		std::move(outputs[1])
	);
}

} // namespace rexlib
