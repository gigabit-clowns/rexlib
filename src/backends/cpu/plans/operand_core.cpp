// SPDX-License-Identifier: GPL-3.0-only

#include "operand_core.hpp"

#include <rexlib/core/layout/strided_layout.hpp>

#include <stdexcept>

namespace rexlib
{
namespace cpu
{

operand_core::operand_core() noexcept
	: m_rank(0)
	, m_extents{}
	, m_strides{}
{
}

operand_core::operand_core(
	std::size_t rank,
	std::array<std::size_t, 3> extents,
	std::array<std::ptrdiff_t, 3> strides
) noexcept
	: m_rank(rank)
	, m_extents(extents)
	, m_strides(strides)
{
}

std::size_t operand_core::get_rank() const noexcept
{
	return m_rank;
}

std::size_t operand_core::get_extent(std::size_t axis) const noexcept
{
	return m_extents[axis];
}

std::ptrdiff_t operand_core::get_stride(std::size_t axis) const noexcept
{
	return m_strides[axis];
}

operand_core split_trailing_core(
	const strided_layout &layout,
	std::size_t core_rank,
	std::vector<std::size_t> &batch_extents,
	std::vector<std::ptrdiff_t> &batch_strides
)
{
	std::vector<std::size_t> extents;
	std::vector<std::ptrdiff_t> strides;
	layout.get_extents(extents);
	layout.get_strides(strides);

	if (extents.size() < core_rank)
	{
		throw std::invalid_argument(
			"operand_core: operand does not have enough axes for its core."
		);
	}

	const auto batch_rank = extents.size() - core_rank;
	batch_extents.assign(extents.begin(), extents.begin() + batch_rank);
	batch_strides.assign(strides.begin(), strides.begin() + batch_rank);

	std::array<std::size_t, 3> core_extents{};
	std::array<std::ptrdiff_t, 3> core_strides{};
	for (std::size_t i = 0; i < core_rank; ++i)
	{
		core_extents[i] = extents[batch_rank + i];
		core_strides[i] = strides[batch_rank + i];
	}
	return operand_core(core_rank, core_extents, core_strides);
}

} // namespace cpu
} // namespace rexlib
