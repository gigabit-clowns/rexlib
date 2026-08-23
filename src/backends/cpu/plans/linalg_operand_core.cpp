// SPDX-License-Identifier: GPL-3.0-only

#include "linalg_operand_core.hpp"

namespace rex
{
namespace cpu
{

linalg_operand_core::linalg_operand_core() noexcept
	: m_rank(0)
	, m_extents{}
	, m_strides{}
{
}

linalg_operand_core::linalg_operand_core(
	std::size_t rank,
	std::array<std::size_t, 2> extents,
	std::array<std::ptrdiff_t, 2> strides
) noexcept
	: m_rank(rank)
	, m_extents(extents)
	, m_strides(strides)
{
}

std::size_t linalg_operand_core::get_rank() const noexcept
{
	return m_rank;
}

std::size_t linalg_operand_core::get_extent(std::size_t axis) const noexcept
{
	return m_extents[axis];
}

std::ptrdiff_t linalg_operand_core::get_stride(std::size_t axis) const noexcept
{
	return m_strides[axis];
}

} // namespace cpu
} // namespace rex
