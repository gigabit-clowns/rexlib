// SPDX-License-Identifier: GPL-3.0-only

#include "frequency_interval.hpp"

namespace rexlib
{
namespace cpu
{

frequency_interval::frequency_interval() noexcept
	: m_lower(0)
	, m_upper(0)
{
}

frequency_interval::frequency_interval(
	std::ptrdiff_t lower,
	std::ptrdiff_t upper
) noexcept
	: m_lower(lower)
	, m_upper(upper)
{
}

std::ptrdiff_t frequency_interval::get_lower() const noexcept
{
	return m_lower;
}

std::ptrdiff_t frequency_interval::get_upper() const noexcept
{
	return m_upper;
}

} // namespace cpu
} // namespace rexlib
