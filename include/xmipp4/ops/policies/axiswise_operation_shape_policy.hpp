// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "axis_list.hpp"

#include <xmipp4/core/dispatch/operation_shape_policy.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>
#include <xmipp4/core/span.hpp>

#include <cstddef>

namespace xmipp4
{
namespace ops
{

/**
 * @brief Shape policy for an operation acting along axes it does not
 * resize.
 *
 * Models operations that move elements about within chosen axes, such as
 * the shift that brings the zero frequency of a spectrum to its centre.
 * The shape comes out as it went in, so the axes affect what the operation
 * does rather than what it produces.
 *
 * They are still the policy's business rather than the operation's own,
 * because they have to be checked against the rank of the operand, and the
 * rank is only known once the operands arrive.
 */
class REXLIB_API axiswise_operation_shape_policy final
	: public operation_shape_policy
{
public:
	/**
	 * @brief Construct a policy acting along a set of axes.
	 *
	 * @param axes The axes acted upon. Sorted on construction.
	 *
	 * @throws std::invalid_argument When an axis is repeated.
	 */
	explicit axiswise_operation_shape_policy(axis_list axes);

	/**
	 * @brief Get the axes acted upon, in ascending order.
	 *
	 * @return span<const std::size_t> The axes.
	 */
	span<const std::size_t> get_axes() const noexcept;

	void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

private:
	XMIPP4_STD_MEMBER_INTERFACE
	axis_list m_axes;
};

} // namespace ops
} // namespace xmipp4
