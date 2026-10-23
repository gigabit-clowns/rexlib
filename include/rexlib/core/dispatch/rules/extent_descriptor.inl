// SPDX-License-Identifier: GPL-3.0-only

#include "extent_descriptor.hpp"

namespace rexlib
{

inline
extent_descriptor::extent_descriptor(
	extent_source source,
	const extent_domain &domain
) noexcept
	: m_source(source)
	, m_domain(domain)
{
}

inline
extent_source extent_descriptor::get_source() const noexcept
{
	return m_source;
}

inline
const extent_domain& extent_descriptor::get_domain() const noexcept
{
	return m_domain;
}

} // namespace rexlib
