// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief A stretch of a mapping, in bytes from its first byte.
 */
class REXLIB_API memory_range
{
public:
	/**
	 * @brief Construct a stretch from its bounds.
	 *
	 * @param offset Where it starts, in bytes from the first byte of the
	 * mapping.
	 * @param size How many bytes it covers.
	 */
	memory_range(std::size_t offset, std::size_t size) noexcept;

	memory_range(const memory_range &other) = default;
	memory_range(memory_range &&other) noexcept = default;
	~memory_range() = default;

	memory_range& operator=(const memory_range &other) = default;
	memory_range& operator=(memory_range &&other) noexcept = default;

	/**
	 * @brief Get where the stretch starts.
	 *
	 * @return std::size_t The offset, in bytes from the first byte of the
	 * mapping.
	 */
	std::size_t get_offset() const noexcept;

	/**
	 * @brief Get how many bytes the stretch covers.
	 *
	 * @return std::size_t The size in bytes.
	 */
	std::size_t get_size() const noexcept;

private:
	std::size_t m_offset;
	std::size_t m_size;
};

/**
 * @brief Ask for stretches of a mapping to be brought into memory.
 *
 * Advice and nothing more: it never fails, and a stretch that was advised may
 * still have to be faulted in when it is read.
 *
 * Every stretch must start on a page boundary and must lie within the
 * mapping. Advice is taken in whole pages, so a stretch starting inside one
 * would reach the bytes before it whether or not it said so; stating the
 * boundary as part of the contract is what lets a caller work its stretches
 * out once, where they are built, rather than have every call walk them again
 * to find out. @ref get_page_size is what the boundary is measured in.
 *
 * Whole batches are taken at once because that is the shape the Windows call
 * wants, where one call serves every stretch.
 *
 * @param base First byte of the mapping.
 * @param ranges The stretches, as byte offsets from @p base.
 */
REXLIB_API
void prefetch_pages(
	byte *base,
	span<const memory_range> ranges
) noexcept;

} // namespace rexlib
