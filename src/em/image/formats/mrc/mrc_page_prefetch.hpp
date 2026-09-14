// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "mrc_byte_range.hpp"

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/platform/operating_system.h>
#include <rexlib/core/span.hpp>

#include <algorithm>
#include <cstddef>

namespace rexlib
{
namespace em
{
namespace mrc
{

/**
 * @brief Clamp a stretch to what is mapped and grow it to whole pages.
 *
 * A page is the unit the operating system advises in, so the stretch grows to
 * the pages its ends fall in rather than shrinking away from them.
 *
 * @param range The stretch, rewritten in place.
 * @param mapped_size How many bytes are mapped.
 * @param page_size Size of a page, which must not be zero.
 * @return bool false if the stretch reaches nothing that is mapped, in which
 * case @p range is left as it was.
 */
inline bool clamp_to_pages(
	mrc_byte_range &range,
	std::size_t mapped_size,
	std::size_t page_size
) noexcept
{
	if (range.byte_offset >= mapped_size || range.byte_size == 0)
	{
		return false;
	}

	const auto available = mapped_size - range.byte_offset;
	const auto last =
		range.byte_offset + std::min(range.byte_size, available);
	const auto first = range.byte_offset - (range.byte_offset % page_size);

	range.byte_offset = first;
	range.byte_size = last - first;

	return true;
}

/**
 * @brief Ask for stretches of a mapping to be brought into memory.
 *
 * Advice and nothing more: it never fails, and a stretch that was advised may
 * still have to be faulted in when it is read. Whole batches are taken at once
 * because that is the shape the Windows call wants, where one call serves
 * every stretch.
 *
 * @param base First byte of the mapping.
 * @param mapped_size How many bytes are mapped.
 * @param ranges The stretches, as byte offsets from @p base. Each is clamped
 * to what is mapped and grown to whole pages.
 */
void prefetch_pages(
	byte *base,
	std::size_t mapped_size,
	span<const mrc_byte_range> ranges
) noexcept;

} // namespace mrc
} // namespace em
} // namespace rexlib

#if REXLIB_POSIX
	#include "mrc_page_prefetch_posix.inl"
#elif REXLIB_WINDOWS
	#include "mrc_page_prefetch_windows.inl"
#else
	#error "No page prefetch implementation available for this platform"
#endif
