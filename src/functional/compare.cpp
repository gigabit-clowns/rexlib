// SPDX-License-Identifier: GPL-3.0-only

#include <rex/functional/compare.hpp>

#include <rex/core/dispatch/execute.hpp>
#include <rex/ops/compare/equal_operation.hpp>
#include <rex/ops/compare/greater_operation.hpp>
#include <rex/ops/compare/greater_equal_operation.hpp>
#include <rex/ops/compare/less_operation.hpp>
#include <rex/ops/compare/less_equal_operation.hpp>
#include <rex/ops/compare/not_equal_operation.hpp>

namespace rexlib
{

array equal(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::equal_operation(), x, y, context, out);
}

array not_equal(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::not_equal_operation(), x, y, context, out);
}

array less(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::less_operation(), x, y, context, out);
}

array less_equal(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::less_equal_operation(), x, y, context, out);
}

array greater(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::greater_operation(), x, y, context, out);
}

array greater_equal(
	const_array_ref x,
	const_array_ref y,
	const execution_context &context,
	array *out
)
{
	return execute_binary(ops::greater_equal_operation(), x, y, context, out);
}

} // namespace rexlib
