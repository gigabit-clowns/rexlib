// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/system/page_prefetch.hpp>

#include <rexlib/core/platform/operating_system.h>

namespace rexlib
{

memory_range::memory_range(std::size_t offset, std::size_t size) noexcept
	: m_offset(offset)
	, m_size(size)
{
}

std::size_t memory_range::get_offset() const noexcept
{
	return m_offset;
}

std::size_t memory_range::get_size() const noexcept
{
	return m_size;
}

} // namespace rexlib

#if REXLIB_POSIX
	#include "page_prefetch_posix.inl"
#elif REXLIB_WINDOWS
	#include "page_prefetch_windows.inl"
#else

	#warning "No page prefetch implementation available for this platform"

namespace rexlib
{

void prefetch_pages(
	byte */*base*/,
	span<const memory_range> /*ranges*/
) noexcept
{
}

} // namespace rexlib

#endif
