// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_page_prefetch.hpp"

#include <sys/mman.h>
#include <unistd.h>

namespace rexlib
{
namespace em
{
namespace mrc
{

namespace detail
{

// POSIX states posix_madvise, which is also the spelling that takes a void*
// everywhere; madvise is the older one, and is what a platform reaching only
// that offers. Where neither is there, a read faults its pages in as it goes.
#if defined(POSIX_MADV_WILLNEED)

inline void advise_will_need(void *address, std::size_t size) noexcept
{
	::posix_madvise(address, size, POSIX_MADV_WILLNEED);
}

#elif defined(MADV_WILLNEED)

inline void advise_will_need(void *address, std::size_t size) noexcept
{
	::madvise(address, size, MADV_WILLNEED);
}

#else

inline void advise_will_need(void *, std::size_t) noexcept
{
}

#endif

} // namespace detail

inline void prefetch_pages(
	byte *base,
	std::size_t mapped_size,
	span<const mrc_byte_range> ranges
) noexcept
{
	const auto reported = ::sysconf(_SC_PAGESIZE);
	if (reported <= 0)
	{
		return;
	}

	const auto page_size = static_cast<std::size_t>(reported);
	for (const auto &range : ranges)
	{
		auto stretch = range;
		if (clamp_to_pages(stretch, mapped_size, page_size))
		{
			detail::advise_will_need(
				static_cast<void*>(base + stretch.byte_offset),
				stretch.byte_size
			);
		}
	}
}

} // namespace mrc
} // namespace em
} // namespace rexlib
