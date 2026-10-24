// SPDX-License-Identifier: GPL-3.0-only

#include "core_dimensions_descriptor.hpp"

namespace rexlib
{

inline
core_dimensions_descriptor::core_dimensions_descriptor(
	span<const extent_slot_descriptor> slots,
	bool has_run,
	const extent_domain &run_domain
) noexcept
	: m_slots(slots)
	, m_has_run(has_run)
	, m_run_domain(run_domain)
{
}

inline
span<const extent_slot_descriptor>
core_dimensions_descriptor::get_slots() const noexcept
{
	return m_slots;
}

inline
bool core_dimensions_descriptor::has_run() const noexcept
{
	return m_has_run;
}

inline
const extent_domain&
core_dimensions_descriptor::get_run_domain() const noexcept
{
	return m_run_domain;
}

} // namespace rexlib
