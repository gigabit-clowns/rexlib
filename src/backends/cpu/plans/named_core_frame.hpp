// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The axes of an operation whose core axes are named, split into
 * core axes and batch axes.
 *
 * The frame is the shape of the result. Its named axes are the core axes,
 * and the rest are the batch axes. Both are held in ascending order,
 * whatever order the axes were named in.
 *
 * An operand is aligned to the frame before its strides are split. An
 * operand carrying the core axes broadcasts to the frame as any operand
 * does. An operand without core axes is all batch: its axes broadcast
 * against the batch axes of the frame.
 */
class named_core_frame
{
public:
	/**
	 * @brief Split a frame into its core axes and its batch axes.
	 *
	 * @param frame_extents The extents of the frame.
	 * @param named_axes The core axes, in any order.
	 * @throws std::out_of_range When a named axis is not within the frame.
	 * @throws std::invalid_argument When an axis is named more than once.
	 */
	named_core_frame(
		std::vector<std::size_t> frame_extents,
		span<const std::size_t> named_axes
	);

	/**
	 * @brief Get the extents of the frame.
	 *
	 * @return span<const std::size_t> One extent per axis.
	 */
	span<const std::size_t> get_frame_extents() const noexcept;

	/**
	 * @brief Get the core axes.
	 *
	 * @return span<const std::size_t> The axes, ascending.
	 */
	span<const std::size_t> get_core_axes() const noexcept;

	/**
	 * @brief Get the batch axes.
	 *
	 * @return span<const std::size_t> The axes, ascending.
	 */
	span<const std::size_t> get_batch_axes() const noexcept;

	/**
	 * @brief Get the extents of the core axes.
	 *
	 * @return std::vector<std::size_t> One extent per core axis, ascending.
	 */
	std::vector<std::size_t> get_core_extents() const;

	/**
	 * @brief Get the extents of the batch axes.
	 *
	 * @return std::vector<std::size_t> One extent per batch axis, ascending.
	 */
	std::vector<std::size_t> get_batch_extents() const;

	/**
	 * @brief Place the axes of an operand in the frame.
	 *
	 * An operand carrying the core axes keeps its axes, aligned to the end
	 * of the frame. The axes of an operand without core axes are aligned to
	 * the end of the batch axes. Every axis of the frame the operand does
	 * not reach gets an extent of 1 and a stride of 0.
	 *
	 * @param layout The layout of the operand.
	 * @param carries_core Whether the operand carries the core axes.
	 * @return strided_layout A layout of the rank of the frame.
	 * @throws std::invalid_argument When the operand has more axes than it
	 * can be placed in.
	 */
	strided_layout embed(
		const strided_layout &layout,
		bool carries_core
	) const;

	/**
	 * @brief Align the layout of an operand to the frame.
	 *
	 * The layout is embedded in the frame, and then broadcast to it.
	 *
	 * @param layout The layout of the operand.
	 * @param carries_core Whether the operand carries the core axes.
	 * @return strided_layout A layout of the extents of the frame, with a
	 * stride of 0 along every axis the operand is broadcast along.
	 * @throws std::invalid_argument When the operand does not broadcast to
	 * the frame.
	 */
	strided_layout align(
		const strided_layout &layout,
		bool carries_core
	) const;

	/**
	 * @brief Select the values of the core axes from values of every axis
	 * of the frame.
	 *
	 * @param frame_values One value per axis of the frame, such as the
	 * strides of an aligned operand.
	 * @return std::vector<std::ptrdiff_t> One value per core axis,
	 * ascending.
	 */
	std::vector<std::ptrdiff_t> select_core(
		span<const std::ptrdiff_t> frame_values
	) const;

	/**
	 * @brief Select the values of the batch axes from values of every axis
	 * of the frame.
	 *
	 * @param frame_values One value per axis of the frame, such as the
	 * strides of an aligned operand.
	 * @return std::vector<std::ptrdiff_t> One value per batch axis,
	 * ascending.
	 */
	std::vector<std::ptrdiff_t> select_batch(
		span<const std::ptrdiff_t> frame_values
	) const;

private:
	std::vector<std::size_t> m_frame_extents;
	std::vector<std::size_t> m_core_axes;
	std::vector<std::size_t> m_batch_axes;
};

} // namespace cpu
} // namespace rexlib
