// SPDX-License-Identifier: GPL-3.0-only

#include "sequence_layout_plan.hpp"

#include <rex/core/layout/strided_layout.hpp>

#include <stdexcept>
#include <vector>

namespace rexlib
{
namespace cpu
{

sequence_layout_plan::sequence_layout_plan(const strided_layout &layout)
	: m_offset(layout.get_offset())
	, m_count(0)
	, m_stride(0)
{
	if (layout.get_rank() != 1)
	{
		throw std::invalid_argument(
			"sequence_layout_plan: a sequence operand must be one "
			"dimensional."
		);
	}

	std::vector<std::size_t> extents;
	std::vector<std::ptrdiff_t> strides;
	layout.get_extents(extents);
	layout.get_strides(strides);

	m_count = extents.front();
	m_stride = strides.front();
}

std::ptrdiff_t sequence_layout_plan::get_offset() const noexcept
{
	return m_offset;
}

std::size_t sequence_layout_plan::get_count() const noexcept
{
	return m_count;
}

std::ptrdiff_t sequence_layout_plan::get_stride() const noexcept
{
	return m_stride;
}

} // namespace cpu
} // namespace rexlib
