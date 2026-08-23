// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/functional/arithmetic.hpp>

#include <xmipp4/core/dispatch/execute.hpp>
#include <xmipp4/core/span.hpp>
#include <xmipp4/ops/arithmetic/abs_operation.hpp>
#include <xmipp4/ops/arithmetic/add_operation.hpp>
#include <xmipp4/ops/arithmetic/divide_operation.hpp>
#include <xmipp4/ops/arithmetic/divmod_operation.hpp>
#include <xmipp4/ops/arithmetic/floor_divide_operation.hpp>
#include <xmipp4/ops/arithmetic/modulo_operation.hpp>
#include <xmipp4/ops/arithmetic/multiply_operation.hpp>
#include <xmipp4/ops/arithmetic/negate_operation.hpp>
#include <xmipp4/ops/arithmetic/sign_operation.hpp>
#include <xmipp4/ops/arithmetic/subtract_operation.hpp>

#include <array>
#include <utility>

namespace rex
{

array add(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::add_operation(), x, y, context, out);
}

array subtract(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::subtract_operation(), x, y, context, out);
}

array negate(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::negate_operation(), x, context, out);
}

array abs(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::abs_operation(), x, context, out);
}

array multiply(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::multiply_operation(), x, y, context, out);
}

array divide(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::divide_operation(), x, y, context, out);
}

array modulo(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::modulo_operation(), x, y, context, out);
}

array floor_divide(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::floor_divide_operation(), x, y, context, out);
}

array sign(
	const_array_ref x,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::sign_operation(), x, context, out);
}

std::pair<array, array> divmod(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *quotient,
	array *remainder
)
{
	// Seed the outputs from whichever the caller supplied, so that their
	// storage is reused. The dispatcher writes back into this array, which
	// is why the results come out of it rather than out of a return value.
	std::array<array, 2> outputs;
	if (quotient)
	{
		outputs[0] = quotient->share();
	}
	if (remainder)
	{
		outputs[1] = remainder->share();
	}

	execute_binary(
		ops::divmod_operation(),
		make_span(outputs),
		x,
		y,
		context
	);

	return std::make_pair(
		std::move(outputs[0]),
		std::move(outputs[1])
	);
}

} // namespace rex
