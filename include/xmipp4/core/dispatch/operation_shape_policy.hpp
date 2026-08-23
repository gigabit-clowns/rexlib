// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operation_descriptor.hpp"

#include <xmipp4/core/span.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>

#include <vector>

namespace rex
{

/**
 * @brief Interface describing the deduction and admissibility of array
 * shapes for an operation.
 *
 * The contract is split in two roles:
 *
 *   * `deduce` is the single source of truth: from the input shapes alone,
 *     it produces the canonical output shapes that the policy would assign,
 *     and along the way performs every input-side check the policy
 *     requires.
 *   * `accept` is invoked only when the user supplies pre-allocated
 *     outputs. It decides whether those user-supplied output shapes are
 *     admissible given the canonical ones produced by `deduce`. The
 *     default implementation requires equality with the canonical shapes;
 *     override only for policies that admit broader-but-compatible user
 *     outputs.
 */
class REXLIB_API operation_shape_policy
{
public:
	using shape_type = std::vector<std::size_t>;

	operation_shape_policy() noexcept;
	operation_shape_policy(const operation_shape_policy &other) = delete;
	operation_shape_policy(operation_shape_policy &&other) = delete;
	virtual ~operation_shape_policy();

	operation_shape_policy&
	operator=(const operation_shape_policy &other) = delete;
	operation_shape_policy&
	operator=(operation_shape_policy &&other) = delete;

	/**
	 * @brief Compute canonical output shapes from the input shapes.
	 *
	 * Performs all input-side validation along the way. After this call,
	 * @p canonical_output_shapes holds the shapes the policy would produce
	 * if the user had not pre-allocated outputs.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param canonical_output_shapes Output buffer, sized to the operation's
	 * output arity. Will be filled with the canonical shapes.
	 * @param input_shapes Shapes of the input operands.
	 */
	virtual void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const = 0;

	/**
	 * @brief Decide whether user-supplied output shapes are admissible.
	 *
	 * Called only when the user has pre-allocated outputs. By the time
	 * this is invoked, `deduce` has already produced
	 * @p canonical_output_shapes, so input-side validation has happened.
	 * Throws on rejection.
	 *
	 * The default implementation requires @p user_output_shapes to be
	 * equal to @p canonical_output_shapes; override only for policies
	 * that admit broader-but-compatible user outputs.
	 *
	 * @param descriptor Description of the operation, used to name the
	 * operation and its operands when reporting a rejection.
	 * @param user_output_shapes Shapes of the user-supplied outputs.
	 * @param canonical_output_shapes Shapes produced by `deduce`.
	 * @param input_shapes Shapes of the input operands.
	 */
	virtual void accept(
		const operation_descriptor &descriptor,
		span<const shape_type> user_output_shapes,
		span<const shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const;
};

} // namespace rex
