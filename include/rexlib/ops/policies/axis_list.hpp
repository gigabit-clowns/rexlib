// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/operation_descriptor.hpp>
#include <rexlib/ops/parameters/axis_list.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace ops
{

/**
 * @brief Put an axis list in the order every reader expects.
 *
 * Sorting lets a policy walk the axes back to front without invalidating
 * the indices it has yet to use, and makes two operations naming the same
 * axes in different orders compare equal.
 *
 * @param axes The axes to be ordered.
 * @return axis_list The axes, in ascending order.
 *
 * @throws std::invalid_argument When an axis is repeated. Naming one twice
 * cannot mean anything, and dropping the duplicate would silently give a
 * result the caller did not ask for.
 */
REXLIB_API
axis_list make_sorted_axis_list(axis_list axes);

/**
 * @brief Check that no axis is named twice, keeping the order given.
 *
 * For an operation that gives one of its axes a role of its own, such as
 * the last one named.
 *
 * @param axes The axes to be checked.
 * @return axis_list The axes, in the order given.
 *
 * @throws std::invalid_argument When an axis is repeated.
 */
REXLIB_API
axis_list make_distinct_axis_list(axis_list axes);

/**
 * @brief Check that every axis names one of the operand's.
 *
 * Separate from @ref make_sorted_axis_list because the rank is not known
 * until the operands arrive, which is long after the operation was built.
 *
 * @param descriptor The operation description, for the diagnostic.
 * @param axes The axes, in any order.
 * @param rank The rank of the operand.
 *
 * @throws std::out_of_range When an axis lies beyond @p rank.
 */
REXLIB_API
void check_axes_within_rank(
	const operation_descriptor &descriptor,
	span<const std::size_t> axes,
	std::size_t rank
);

} // namespace ops
} // namespace rexlib
