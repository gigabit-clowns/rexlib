// SPDX-License-Identifier: GPL-3.0-only

#include "type_pivot_descriptor.hpp"

namespace rexlib
{

inline
type_pivot_descriptor::type_pivot_descriptor(
	type_pivot_source source,
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
type_pivot_source type_pivot_descriptor::get_source() const noexcept
{
	return m_source;
}

inline
std::size_t type_pivot_descriptor::get_index() const noexcept
{
	return m_index;
}

inline
std::ptrdiff_t type_pivot_descriptor::get_default_pivot() const noexcept
{
	return m_default_pivot;
}

inline
const numerical_type_domain& type_pivot_descriptor::get_domain() const noexcept
{
	return m_domain.get();
}

} // namespace rexlib
