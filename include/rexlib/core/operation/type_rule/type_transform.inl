// SPDX-License-Identifier: GPL-3.0-only

#include "type_transform.hpp"

#include <rexlib/core/platform/assert.hpp>

namespace rexlib
{

inline
type_transform::type_transform(
	apply_function apply,
	const numerical_type_domain &domain,
	const char *relation
) noexcept
	: m_apply(apply)
	, m_domain(domain)
	, m_relation(relation)
{
	REXLIB_ASSERT(m_apply);
	REXLIB_ASSERT(m_relation);
}

inline
numerical_type type_transform::apply(numerical_type type) const noexcept
{
	return m_apply(type);
}

inline
const numerical_type_domain& type_transform::get_domain() const noexcept
{
	return m_domain.get();
}

inline
const char* type_transform::get_relation() const noexcept
{
	return m_relation;
}

template <typename Transform>
inline
type_transform make_type_transform() noexcept
{
	return type_transform(
		&Transform::apply,
		Transform::domain_type::get(),
		Transform::get_relation()
	);
}

} // namespace rexlib
