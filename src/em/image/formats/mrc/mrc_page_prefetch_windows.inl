// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_page_prefetch.hpp"

#include <rexlib/core/platform/constexpr.hpp>

#include <windows.h>

#include <array>

namespace rexlib
{
namespace em
{
namespace mrc
{

namespace detail
{

// The layout of WIN32_MEMORY_RANGE_ENTRY, stated here rather than taken from
// the headers, which declare it only for Windows 8 and newer.
struct memory_range_entry
{
	void *address;
	SIZE_T size;
};

using prefetch_virtual_memory_function = BOOL (WINAPI *)(
	HANDLE,
	ULONG_PTR,
	memory_range_entry*,
	ULONG
);

// PrefetchVirtualMemory arrived in Windows 8, so it is resolved at run time:
// where it is missing the mapping is read as it always was, one fault at a
// time. kernel32 is loaded into every process, so the handle is never taken.
inline prefetch_virtual_memory_function
get_prefetch_virtual_memory() noexcept
{
	static const auto function =
		reinterpret_cast<prefetch_virtual_memory_function>(
			::GetProcAddress(
				::GetModuleHandleW(L"kernel32.dll"),
				"PrefetchVirtualMemory"
			)
		);

	return function;
}

} // namespace detail

inline void prefetch_pages(
	byte *base,
	std::size_t mapped_size,
	span<const mrc_byte_range> ranges
) noexcept
{
	const auto prefetch = detail::get_prefetch_virtual_memory();
	if (prefetch == nullptr)
	{
		return;
	}

	SYSTEM_INFO information;
	::GetSystemInfo(&information);

	const auto page_size = static_cast<std::size_t>(information.dwPageSize);
	if (page_size == 0)
	{
		return;
	}

	// Gathered a fixed batch at a time rather than into one allocation, so
	// that advising cannot throw where it is only a hint.
	REXLIB_CONST_CONSTEXPR std::size_t batch_size = 64;
	std::array<detail::memory_range_entry, batch_size> entries;
	std::size_t count = 0;

	for (const auto &range : ranges)
	{
		auto stretch = range;
		if (!clamp_to_pages(stretch, mapped_size, page_size))
		{
			continue;
		}

		entries[count].address =
			static_cast<void*>(base + stretch.byte_offset);
		entries[count].size = static_cast<SIZE_T>(stretch.byte_size);
		++count;

		if (count == batch_size)
		{
			prefetch(::GetCurrentProcess(), count, entries.data(), 0);
			count = 0;
		}
	}

	if (count > 0)
	{
		prefetch(::GetCurrentProcess(), count, entries.data(), 0);
	}
}

} // namespace mrc
} // namespace em
} // namespace rexlib
