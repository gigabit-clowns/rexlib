// SPDX-License-Identifier: GPL-3.0-only

#include "shape_rule_resolution.hpp"

#include <rexlib/core/platform/assert.hpp>

#include <utility>

namespace rexlib
{

shape_rule_resolution::shape_rule_resolution(
	extent_list batch,
	extent_list extents,
	extent_list run_extents
) noexcept
	: m_batch(std::move(batch))
	, m_extents(std::move(extents))
	, m_run_extents(std::move(run_extents))
{
}

const shape_rule_resolution::extent_list&
shape_rule_resolution::get_batch() const noexcept
{
	return m_batch;
}

std::size_t shape_rule_resolution::get_extent(std::size_t index) const noexcept
{
	REXLIB_ASSERT(index < m_extents.size());
	return m_extents[index];
}

const shape_rule_resolution::extent_list&
shape_rule_resolution::get_run_extents() const noexcept
{
	return m_run_extents;
}

} // namespace rexlib
