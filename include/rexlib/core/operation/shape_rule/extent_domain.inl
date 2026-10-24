// SPDX-License-Identifier: GPL-3.0-only

#include "extent_domain.hpp"

namespace rexlib
{

inline
extent_domain::extent_domain(
	contains_function_type contains,
	const char *description
) noexcept
	: m_contains(contains)
	, m_description(description)
{
}

inline
bool extent_domain::contains(std::size_t extent) const noexcept
{
	return m_contains(extent);
}

inline
const char* extent_domain::get_description() const noexcept
{
	return m_description;
}

template <typename Domain>
inline
extent_domain make_extent_domain() noexcept
{
	return extent_domain(&Domain::contains, Domain::get_description());
}

} // namespace rexlib
