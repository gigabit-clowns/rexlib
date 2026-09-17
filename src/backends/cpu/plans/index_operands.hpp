// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/element_index_tags.hpp>

#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{

class joint_layout_builder;

namespace cpu
{

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
