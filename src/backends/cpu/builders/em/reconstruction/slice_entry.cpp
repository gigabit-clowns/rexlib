// SPDX-License-Identifier: GPL-3.0-only

#include "slice_entry.hpp"

namespace rexlib
{
namespace cpu
{

slice_entry::slice_entry() noexcept
	: m_frame({{ 0, 0, 0 }}, {{ 0, 0, 0 }}, 0.0)
	, m_last_shell(-1)
	, m_slice_offset(0)
	, m_radial_offset(0)
	, m_phase_offset(0)
{
}

slice_entry::slice_entry(
	const slice_frame &frame,
	std::ptrdiff_t last_shell,
	std::ptrdiff_t slice_offset,
	std::ptrdiff_t radial_offset,
	std::size_t phase_offset
) noexcept
	: m_frame(frame)
	, m_last_shell(last_shell)
	, m_slice_offset(slice_offset)
	, m_radial_offset(radial_offset)
	, m_phase_offset(phase_offset)
{
}

const slice_frame& slice_entry::get_frame() const noexcept
{
	return m_frame;
}

std::ptrdiff_t slice_entry::get_last_shell() const noexcept
{
	return m_last_shell;
}

std::ptrdiff_t slice_entry::get_slice_offset() const noexcept
{
	return m_slice_offset;
}

std::ptrdiff_t slice_entry::get_radial_offset() const noexcept
{
	return m_radial_offset;
}

std::size_t slice_entry::get_phase_offset() const noexcept
{
	return m_phase_offset;
}

} // namespace cpu
} // namespace rexlib
