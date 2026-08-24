// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <array>
#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The raw, un-reordered core axes of one matmul/matvec/vecmat/cross
 * operand.
 *
 * Never merged into a batch joint_layout: a kernel receives it directly and
 * decides what to do with it (a GEMM/GEMV size and layout dispatch, or a
 * cross product formula). Its rank is one or two.
 *
 * @see linalg_core_layout_plan
 */
class linalg_operand_core
{
public:
	linalg_operand_core() noexcept;

	linalg_operand_core(
		std::size_t rank,
		std::array<std::size_t, 2> extents,
		std::array<std::ptrdiff_t, 2> strides
	) noexcept;

	std::size_t get_rank() const noexcept;
	std::size_t get_extent(std::size_t axis) const noexcept;
	std::ptrdiff_t get_stride(std::size_t axis) const noexcept;

private:
	std::size_t m_rank;
	std::array<std::size_t, 2> m_extents;
	std::array<std::ptrdiff_t, 2> m_strides;
};

} // namespace cpu
} // namespace rexlib
