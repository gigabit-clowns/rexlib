// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "axis_list.hpp"

#include <rex/core/dispatch/operation_shape_policy.hpp>
#include <rex/core/platform/dynamic_shared_object.h>
#include <rex/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rex
{
namespace ops
{

/**
 * @brief Shape policy for reductions over a set of axes.
 *
 * The inputs are broadcast together first and the reduced axes are then
 * dropped, or kept with an extent of one. Broadcasting first is what lets
 * one policy serve both a plain reduction and one taking a mask alongside
 * its operand: the mask only has to be broadcast compatible with it.
 *
 * The axes are explicit, non negative and known at construction. Reducing
 * over every axis is not a mode of this policy: it needs a rank to mean
 * anything, and the caller that has the array knows the rank, so the
 * functional layer spells it out into a list before an operation is ever
 * built. An operation is a rigid description of work, not a request to be
 * interpreted later.
 */
class REXLIB_API reduction_operation_shape_policy final
	: public operation_shape_policy
{
public:
	using axis_list_type = axis_list;

	/**
	 * @brief Construct a policy reducing over a set of axes.
	 *
	 * @param axes The axes to reduce over. Sorted on construction, so that
	 * the deduction and any backend see them in one order. An empty list
	 * reduces over no axis, leaving the shape unchanged.
	 * @param keep_dimensions Whether the reduced axes are kept with an
	 * extent of one instead of being dropped.
	 *
	 * @throws std::invalid_argument When an axis is repeated.
	 */
	explicit reduction_operation_shape_policy(
		axis_list_type axes,
		bool keep_dimensions = false
	);

	/**
	 * @brief Get the axes being reduced, in ascending order.
	 *
	 * @return span<const std::size_t> The axes.
	 */
	span<const std::size_t> get_axes() const noexcept;

	/**
	 * @brief Check whether the reduced axes are kept.
	 *
	 * @return bool The keep dimensions flag.
	 */
	bool get_keep_dimensions() const noexcept;

	void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

private:
	REXLIB_STD_MEMBER_INTERFACE
	axis_list_type m_axes;
	bool m_keep_dimensions;
};

} // namespace ops
} // namespace rex
