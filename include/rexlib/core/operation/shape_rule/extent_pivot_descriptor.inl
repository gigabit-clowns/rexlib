// SPDX-License-Identifier: GPL-3.0-only

#include "extent_pivot_descriptor.hpp"

namespace rexlib
{

inline
extent_pivot_descriptor::extent_pivot_descriptor(
	extent_pivot_source source,
	const extent_domain &domain
) noexcept
	: m_source(source)
	, m_domain(domain)
{
}

inline
extent_pivot_source extent_pivot_descriptor::get_source() const noexcept
{
	return m_source;
}

inline
const extent_domain& extent_pivot_descriptor::get_domain() const noexcept
{
	return m_domain;
}

} // namespace rexlib
