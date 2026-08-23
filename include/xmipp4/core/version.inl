// SPDX-License-Identifier: GPL-3.0-only

#include "version.hpp"

namespace xmipp4 
{

REX_INLINE_CONSTEXPR 
version::version(
	std::uint32_t major, 
	std::uint32_t minor, 
	std::uint32_t patch
) noexcept
	: m_data(major) // No need to mask, as it will be shifted
{
	// Shift and write fields
	REX_CONST_CONSTEXPR std::uint32_t right_minor_mask = (1 << minor_bits) - 1;
	m_data <<= minor_bits;
	m_data |= minor & right_minor_mask;

	REX_CONST_CONSTEXPR std::uint32_t right_patch_mask = (1 << patch_bits) - 1;
	m_data <<= patch_bits;
	m_data |= patch & right_patch_mask;
}

REX_INLINE_CONSTEXPR 
void version::set_major(std::uint32_t major) noexcept
{
	major <<= major_offset;
	m_data &= ~major_mask;
	m_data |= major;
}

REX_INLINE_CONSTEXPR 
std::uint32_t version::get_major() const noexcept
{
	return m_data >> major_offset;
}

REX_INLINE_CONSTEXPR 
void version::set_minor(std::uint32_t minor) noexcept
{
	minor <<= minor_offset;
	minor &= minor_mask;
	m_data &= ~minor_mask;
	m_data |= minor;
}

REX_INLINE_CONSTEXPR 
std::uint32_t version::get_minor() const noexcept
{
	return (m_data & minor_mask) >> minor_bits;
}

REX_INLINE_CONSTEXPR 
void version::set_patch(std::uint32_t patch) noexcept
{
	patch &= patch_mask;
	m_data &= ~patch_mask;
	m_data |= patch;
}

REX_INLINE_CONSTEXPR 
std::uint32_t version::get_patch() const noexcept
{
	return m_data & patch_mask;
}

REX_INLINE_CONSTEXPR 
std::uint32_t version::get_data() const noexcept
{
	return m_data;
}

} // namespace xmipp4
