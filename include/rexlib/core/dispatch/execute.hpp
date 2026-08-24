// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/dynamic_shared_object.h"
#include "../span.hpp"

namespace rexlib
{

class array;
class const_array_ref;


class operation;
class execution_context;

/**
 * @brief Execute a operation.
 *
 * @param operation The operation to be executed.
 * @param output_operands The output operands.
 * @param input_operands The input operands.
 * @param context The execution context used for dispatching.
 */
REXLIB_API
void execute(
	const operation &operation,
	span<array> output_operands,
	span<const const_array_ref> input_operands,
	const execution_context &context
);

/**
 * @brief Execute a operation.
 *
 * @param operation The operation to be executed.
 * @param input_operands The input operands.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 */
REXLIB_API
array execute(
	const operation &operation,
	span<const const_array_ref> input_operands,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Execute an unary operation.
 *
 * @param operation The operation to be executed.
 * @param input The input operand.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 */
REXLIB_API
array execute_unary(
	const operation &operation,
	const const_array_ref &input,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Execute an unary operation producing several outputs.
 *
 * Unlike the single output form there is nothing to return: an output that
 * already holds storage is reused and one that does not is allocated and
 * written back in place, so the results are delivered through
 * @p output_operands.
 *
 * @param operation The operation to be executed.
 * @param output_operands The output operands, in signature order.
 * @param input The input operand.
 * @param context The execution context used for dispatching.
 */
REXLIB_API
void execute_unary(
	const operation &operation,
	span<array> output_operands,
	const_array_ref input,
	const execution_context &context
);

/**
 * @brief Execute an unary operation.
 *
 * @param operation The operation to be executed.
 * @param first_input The first input operand.
 * @param second_input The second input operand.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 */
REXLIB_API
array execute_binary(
	const operation &operation,
	const_array_ref first_input,
	const_array_ref second_input,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Execute a binary operation producing several outputs.
 *
 * @param operation The operation to be executed.
 * @param output_operands The output operands, in signature order.
 * @param first_input The first input operand.
 * @param second_input The second input operand.
 * @param context The execution context used for dispatching.
 *
 * @see execute_unary
 */
REXLIB_API
void execute_binary(
	const operation &operation,
	span<array> output_operands,
	const_array_ref first_input,
	const_array_ref second_input,
	const execution_context &context
);

/**
 * @brief Execute an unary operation.
 *
 * @param operation The operation to be executed.
 * @param first_input The first input operand.
 * @param second_input The second input operand.
 * @param third_input The third input operand.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 */
REXLIB_API
array execute_ternary(
	const operation &operation,
	const_array_ref first_input,
	const_array_ref second_input,
	const_array_ref third_input,
	const execution_context &context,
	array *out = nullptr
);

} // namespace rexlib
