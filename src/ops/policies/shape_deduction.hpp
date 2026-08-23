// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/dispatch/operation_descriptor.hpp>
#include <xmipp4/core/dispatch/operation_shape_policy.hpp>
#include <xmipp4/core/span.hpp>

namespace rex
{
namespace ops
{

/**
 * @brief The shape of an operand, as a shape policy spells it.
 */
using shape_type = operation_shape_policy::shape_type;

/**
 * @brief The shape every input operand broadcasts to.
 *
 * Where a policy that reads its shape off its operands starts, whatever it
 * goes on to do with it. Broadcasting before anything else is what lets one
 * policy serve an operation taking a mask, or a stack, alongside its operand:
 * the companion only has to be broadcast compatible with it.
 *
 * @param descriptor The operation description, for the diagnostic.
 * @param input_shapes Shapes of the input operands.
 * @return shape_type The broadcast shape.
 *
 * @throws std::invalid_argument When there is no input operand to take a
 * shape from, or when the inputs are not broadcast compatible.
 */
shape_type broadcast_input_shapes(
	const operation_descriptor &descriptor,
	span<const shape_type> input_shapes
);

/**
 * @brief Give every output slot the shape that was deduced.
 *
 * Where a policy ends. Every output of a multi output operation has the same
 * shape in the whole of this catalog, so a policy deduces one shape and hands
 * it over here rather than filling the slots itself.
 *
 * @param canonical_output_shapes The slots to fill, sized to the operation's
 * output arity. An operation with no output is admissible and leaves the
 * shape unused.
 * @param shape The deduced shape. Taken by value and moved into the last
 * slot, the others being served from it first.
 */
void assign_output_shapes(
	span<shape_type> canonical_output_shapes,
	shape_type shape
);

} // namespace ops
} // namespace rex
