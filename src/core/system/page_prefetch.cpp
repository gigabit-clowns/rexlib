// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/system/page_prefetch.hpp>

#include <rexlib/core/platform/operating_system.h>

namespace rexlib
{

memory_range::memory_range(void *address, std::size_t size) noexcept
	: m_address(address)
	, m_size(size)
{
}

void* memory_range::get_address() const noexcept
{
	return m_address;
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

void prefetch_pages(span<const memory_range> /*ranges*/) noexcept
{
}

} // namespace rexlib

#endif
