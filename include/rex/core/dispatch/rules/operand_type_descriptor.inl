// SPDX-License-Identifier: GPL-3.0-only

#include "operand_type_descriptor.hpp"

#include <rex/core/platform/assert.hpp>

namespace rex
{

inline
pivot_descriptor::pivot_descriptor(
	pivot_source source,
	std::size_t index,
	std::ptrdiff_t default_pivot,
	const numerical_type_domain &domain
) noexcept
	: m_source(source)
	, m_index(index)
	, m_default_pivot(default_pivot)
	, m_domain(domain)
{
}

inline
pivot_source pivot_descriptor::get_source() const noexcept
{
	return m_source;
}

inline
std::size_t pivot_descriptor::get_index() const noexcept
{
	return m_index;
}

inline
std::ptrdiff_t pivot_descriptor::get_default_pivot() const noexcept
{
	return m_default_pivot;
}

inline
const numerical_type_domain& pivot_descriptor::get_domain() const noexcept
{
	return m_domain.get();
}

inline
slot_descriptor::slot_descriptor(
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
std::size_t slot_descriptor::get_pivot_index() const noexcept
{
	return m_pivot_index;
}

inline
numerical_type slot_descriptor::apply(numerical_type type) const noexcept
{
	return m_apply(type);
}

inline
const numerical_type_domain&
slot_descriptor::get_transform_domain() const noexcept
{
	return m_transform_domain.get();
}

inline
const char* slot_descriptor::get_relation() const noexcept
{
	return m_relation;
}

} // namespace rex
