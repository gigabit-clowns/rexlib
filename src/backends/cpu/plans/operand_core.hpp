// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/layout/strided_layout.hpp>

#include <array>
#include <cstddef>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The raw, un-reordered core axes of one operand.
 *
 * Never merged into a batch joint_layout: a kernel receives it directly and
 * decides what to do with it (a GEMM/GEMV size and layout dispatch, a cross
 * product formula, a volume to insert into). Its rank is at most three.
 *
 * @see linalg_core_layout_plan
 * @see slice_insertion_layout_plan
 */
class operand_core
{
public:
	operand_core() noexcept;

	operand_core(
		std::size_t rank,
		std::array<std::size_t, 3> extents,
		std::array<std::ptrdiff_t, 3> strides
	) noexcept;

	std::size_t get_rank() const noexcept;
	std::size_t get_extent(std::size_t axis) const noexcept;
	std::ptrdiff_t get_stride(std::size_t axis) const noexcept;

private:
	std::size_t m_rank;
	std::array<std::size_t, 3> m_extents;
	std::array<std::ptrdiff_t, 3> m_strides;
};

/**
 * @brief Split the trailing axes of a layout off as a core, leaving the rest
 * as the batch prefix.
 *
 * @param layout The layout of the operand.
 * @param core_rank The number of trailing axes that are core. At most
 * three.
 * @param batch_extents Receives the extents of the other axes.
 * @param batch_strides Receives the strides of the other axes.
 * @return operand_core The core.
 *
 * @throws std::invalid_argument When the layout has fewer axes than the
 * core.
 */
operand_core split_trailing_core(
	const strided_layout &layout,
	std::size_t core_rank,
	std::vector<std::size_t> &batch_extents,
	std::vector<std::ptrdiff_t> &batch_strides
);

} // namespace cpu
} // namespace rexlib
