// SPDX-License-Identifier: GPL-3.0-only

#include "type_slot_descriptor.hpp"

#include <rexlib/core/platform/assert.hpp>

namespace rexlib
{

inline
type_slot_descriptor::type_slot_descriptor(
	std::size_t pivot_index,
	transform_function apply,
	const numerical_type_domain &transform_domain,
	const char *relation
) noexcept
	: m_pivot_index(pivot_index)
	, m_apply(apply)
	, m_transform_domain(transform_domain)
	, m_relation(relation)
{
	REXLIB_ASSERT(m_apply);
	REXLIB_ASSERT(m_relation);
}

inline
std::size_t type_slot_descriptor::get_pivot_index() const noexcept
{
	return m_pivot_index;
}

inline
numerical_type type_slot_descriptor::apply(numerical_type type) const noexcept
{
	return m_apply(type);
}

inline
const numerical_type_domain&
type_slot_descriptor::get_transform_domain() const noexcept
{
	return m_transform_domain.get();
}

inline
const char* type_slot_descriptor::get_relation() const noexcept
{
	return m_relation;
}

} // namespace rexlib
