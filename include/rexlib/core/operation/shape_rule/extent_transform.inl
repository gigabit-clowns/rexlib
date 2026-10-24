// SPDX-License-Identifier: GPL-3.0-only

#include "extent_transform.hpp"

#include <rexlib/core/operation/shape_rule/same_extent_transform.hpp>
#include <rexlib/core/platform/assert.hpp>

#include <type_traits>

namespace rexlib
{

inline
extent_transform::extent_transform(
	apply_function apply,
	const char *relation,
	bool identity
) noexcept
	: m_apply(apply)
	, m_relation(relation)
	, m_identity(identity)
{
	REXLIB_ASSERT(m_apply);
	REXLIB_ASSERT(m_relation);
}

inline
std::size_t extent_transform::apply(std::size_t extent) const noexcept
{
	return m_apply(extent);
}

inline
const char* extent_transform::get_relation() const noexcept
{
	return m_relation;
}

inline
bool extent_transform::is_identity() const noexcept
{
	return m_identity;
}

template <typename Transform>
inline
extent_transform make_extent_transform() noexcept
{
	return extent_transform(
		&Transform::apply,
		Transform::get_relation(),
		std::is_same<Transform, same_extent_transform>::value
	);
}

} // namespace rexlib
