// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief A stretch of memory: where it starts and how many bytes it covers.
 */
class REXLIB_API memory_range
{
public:
	/**
	 * @brief Construct a stretch from its first byte and its size.
	 *
	 * @param address First byte of the stretch.
	 * @param size How many bytes it covers.
	 */
	memory_range(void *address, std::size_t size) noexcept;

	memory_range(const memory_range &other) = default;
	memory_range(memory_range &&other) noexcept = default;
	~memory_range() = default;

	memory_range& operator=(const memory_range &other) = default;
	memory_range& operator=(memory_range &&other) noexcept = default;

	/**
	 * @brief Get the first byte of the stretch.
	 *
	 * @return void* Its address.
	 */
	void* get_address() const noexcept;

	/**
	 * @brief Get how many bytes the stretch covers.
	 *
	 * @return std::size_t The size in bytes.
	 */
	std::size_t get_size() const noexcept;

private:
	void *m_address;
	std::size_t m_size;
};

/**
 * @brief Ask for stretches of mapped memory to be paged in.
 *
 * Advice and nothing more: it never fails, and a stretch that was advised may
 * still have to be faulted in when it is read.
 *
 * Every stretch must start at the first byte of a page and lie within memory
 * that is mapped. Advice is taken in whole pages, so a stretch starting
 * inside one would reach the bytes before it whether or not it said so;
 * stating the boundary as part of the contract is what lets a caller work its
 * stretches out once, where they are built, rather than have every call walk
 * them again to find out. @ref get_page_size is what the boundary is measured
 * in.
 *
 * Whole batches are taken at once because that is the shape the Windows call
 * wants, where one call serves every stretch. The stretches of one batch need
 * not belong to the same mapping.
 *
 * @param ranges The stretches to advise.
 */
REXLIB_API
void prefetch_pages(span<const memory_range> ranges) noexcept;

} // namespace rexlib
