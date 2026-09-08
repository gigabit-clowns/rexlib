// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{
namespace cuda
{

/**
 * @brief Where a caching allocator gets its raw memory from.
 *
 * One call per heap, never per allocation: the caching layer above reaches the
 * source only when its pool has nothing left to hand out. That is what lets a
 * single caching engine serve both device-local and page-locked host memory
 * without either of them leaking into it.
 */
class memory_source
{
public:
	memory_source() noexcept = default;
	memory_source(const memory_source &other) = delete;
	memory_source(memory_source &&other) = delete;
	virtual ~memory_source();

	memory_source& operator=(const memory_source &other) = delete;
	memory_source& operator=(memory_source &&other) = delete;

	/**
	 * @brief Obtain a new region of memory.
	 *
	 * @param size Size of the region, in bytes. Must be greater than zero.
	 * @return void* Pointer to the region. Never null. Aligned to at least
	 * @ref get_base_alignment.
	 *
	 * @throws std::bad_alloc If there was not enough memory left.
	 * @throws error If the region could not be obtained for any other reason.
	 */
	virtual void* allocate(std::size_t size) = 0;

	/**
	 * @brief Return a region obtained from @ref allocate.
	 *
	 * Runs where a failure can no longer be handled, so it reports rather than
	 * throws.
	 *
	 * @param data Pointer returned by @ref allocate. May be null.
	 */
	virtual void deallocate(void *data) noexcept = 0;

	/**
	 * @brief Check whether the host can address what this source hands out.
	 *
	 * The source is what actually took the memory, so it is what knows this;
	 * the kind its resource advertises is a description of the same fact
	 * rather than the fact itself.
	 *
	 * @return true The host can read and write the memory directly.
	 * @return false The memory has to be transferred to reach the host.
	 */
	virtual bool is_host_accessible() const noexcept = 0;

	/**
	 * @brief Get the alignment that every region from @ref allocate satisfies.
	 *
	 * @return std::size_t Alignment, in bytes. A power of two, constant over
	 * the lifetime of the source.
	 */
	virtual std::size_t get_base_alignment() const noexcept = 0;
};

} // namespace cuda
} // namespace rexlib
