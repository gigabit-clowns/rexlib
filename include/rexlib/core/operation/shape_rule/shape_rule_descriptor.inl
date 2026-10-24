// SPDX-License-Identifier: GPL-3.0-only

#include "shape_rule_descriptor.hpp"

namespace rexlib
{

inline
shape_rule_descriptor::shape_rule_descriptor(
	span<const extent_pivot_descriptor> extents,
	span<const core_dimensions_descriptor> output_cores,
	span<const core_dimensions_descriptor> input_cores,
	core_placement placement,
	output_batch batch
) noexcept
	: m_extents(extents)
	, m_output_cores(output_cores)
	, m_input_cores(input_cores)
	, m_placement(placement)
	, m_output_batch(batch)
{
}

inline
span<const extent_pivot_descriptor>
shape_rule_descriptor::get_extents() const noexcept
{
	return m_extents;
}

inline
span<const core_dimensions_descriptor>
shape_rule_descriptor::get_output_cores() const noexcept
{
	return m_output_cores;
}

inline
span<const core_dimensions_descriptor>
shape_rule_descriptor::get_input_cores() const noexcept
{
	return m_input_cores;
}

inline
core_placement shape_rule_descriptor::get_placement() const noexcept
{
	return m_placement;
}

inline
output_batch shape_rule_descriptor::get_output_batch() const noexcept
{
	return m_output_batch;
}

} // namespace rexlib
