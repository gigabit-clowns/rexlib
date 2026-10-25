// SPDX-License-Identifier: GPL-3.0-only

#include "type_rule_descriptor.hpp"

namespace rexlib
{

inline
type_rule_descriptor::type_rule_descriptor(
	span<const type_pivot_descriptor> pivots,
	span<const type_slot_descriptor> output_slots,
	span<const type_slot_descriptor> input_slots
) noexcept
	: m_pivots(pivots)
	, m_output_slots(output_slots)
	, m_input_slots(input_slots)
{
}

inline
span<const type_pivot_descriptor>
type_rule_descriptor::get_pivots() const noexcept
{
	return m_pivots;
}

inline
span<const type_slot_descriptor>
type_rule_descriptor::get_output_slots() const noexcept
{
	return m_output_slots;
}

inline
span<const type_slot_descriptor>
type_rule_descriptor::get_input_slots() const noexcept
{
	return m_input_slots;
}

} // namespace rexlib
