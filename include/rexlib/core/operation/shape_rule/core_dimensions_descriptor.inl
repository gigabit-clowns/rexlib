// SPDX-License-Identifier: GPL-3.0-only

#include "core_dimensions_descriptor.hpp"

#include <algorithm>
#include <iterator>

namespace rexlib
{

inline
core_dimensions_descriptor::core_dimensions_descriptor(
	span<const axis_slot_descriptor> slots
) noexcept
	: m_slots(slots)
	, m_run_position(
		static_cast<std::size_t>(
			std::distance(
				slots.begin(),
				std::find_if(
					slots.begin(),
					slots.end(),
					[] (const axis_slot_descriptor &slot)
					{
						return slot.get_kind() == axis_slot_kind::run;
					}
				)
			)
		)
	)
{
}

inline
span<const axis_slot_descriptor>
core_dimensions_descriptor::get_slots() const noexcept
{
	return m_slots;
}

inline
bool core_dimensions_descriptor::has_run() const noexcept
{
	return m_run_position < m_slots.size();
}

inline
std::size_t core_dimensions_descriptor::get_run_position() const noexcept
{
	return m_run_position;
}

inline
std::size_t core_dimensions_descriptor::get_axis_count() const noexcept
{
	return has_run() ? m_slots.size() - 1 : m_slots.size();
}

} // namespace rexlib
