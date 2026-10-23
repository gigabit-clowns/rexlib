// SPDX-License-Identifier: GPL-3.0-only

#include "axis_slot_descriptor.hpp"

namespace rexlib
{

inline
axis_slot_descriptor::axis_slot_descriptor(
	axis_slot_kind kind,
	std::size_t index,
	transform_function apply,
	const char *relation,
	const extent_domain &domain
) noexcept
	: m_kind(kind)
	, m_index(index)
	, m_apply(apply)
	, m_relation(relation)
	, m_domain(domain)
{
}

inline
axis_slot_kind axis_slot_descriptor::get_kind() const noexcept
{
	return m_kind;
}

inline
std::size_t axis_slot_descriptor::get_index() const noexcept
{
	return m_index;
}

inline
std::size_t axis_slot_descriptor::apply(std::size_t extent) const noexcept
{
	return m_apply(extent);
}

inline
const char* axis_slot_descriptor::get_relation() const noexcept
{
	return m_relation;
}

inline
const extent_domain& axis_slot_descriptor::get_domain() const noexcept
{
	return m_domain;
}

} // namespace rexlib
