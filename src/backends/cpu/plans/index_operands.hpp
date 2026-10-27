// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/element_index_tags.hpp>

#include <rexlib/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{

class joint_layout_builder;

namespace cpu
{

/**
 * @brief Compute the row-major strides of the linear index over every axis.
 *
 * @param extents Extents of the index space, in axis order.
 * @return std::vector<std::ptrdiff_t> One stride per axis, in elements.
 */
std::vector<std::ptrdiff_t> make_row_major_strides(
	span<const std::size_t> extents
);

/**
 * @brief Compute the row-major strides of the linear index over some of
 * the axes.
 *
 * The index counts the positions along the counted axes only: an axis that
 * is not counted has a stride of 0, and contributes no extent to the
 * strides of the axes before it.
 *
 * @param extents Extents of the index space, in axis order.
 * @param counted_axes The axes counted, ascending and without repetitions.
 * @return std::vector<std::ptrdiff_t> One stride per axis, in elements.
 * @throws std::out_of_range When a counted axis is not within the rank.
 */
std::vector<std::ptrdiff_t> make_row_major_strides(
	span<const std::size_t> extents,
	span<const std::size_t> counted_axes
);

/**
 * @brief Add the index operands of a traversal that hands out no index,
 * which is none.
 *
 * @param builder The builder.
 * @param extents Extents of the index space.
 */
void add_index_operands(
	joint_layout_builder &builder,
	span<const std::size_t> extents,
	no_index_tag
) noexcept;

/**
 * @brief Add the operand whose offset is the linear index of the element
 * being visited.
 *
 * The operand has row-major strides over @p extents and no offset, and the
 * builder reorders and coalesces it together with every other operand, so its
 * offset stays the linear index whatever order the axes are traversed in.
 *
 * @param builder The builder. Its iteration space must have @p extents, and
 * it must already hold every other operand.
 * @param extents Extents of the index space, in axis order.
 */
void add_index_operands(
	joint_layout_builder &builder,
	span<const std::size_t> extents,
	linear_index_tag
);

/**
 * @brief Add one operand per axis whose offset is the coordinate of the
 * element being visited along that axis.
 *
 * The operand of an axis strides by one along it and not at all along any
 * other, and has no offset. The operands are added in axis order.
 *
 * @param builder The builder. Its iteration space must have @p extents, and
 * it must already hold every other operand.
 * @param extents Extents of the index space, in axis order.
 */
void add_index_operands(
	joint_layout_builder &builder,
	span<const std::size_t> extents,
	multidimensional_index_tag
);

} // namespace cpu
} // namespace rexlib
