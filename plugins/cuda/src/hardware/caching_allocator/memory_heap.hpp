// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{
namespace cuda
{

class memory_source;

/**
 * @brief One region taken from a @ref memory_source, to be carved into blocks.
 *
 * The heap is what a caching pool actually asks the driver for. Its base is
 * aligned to whatever the pool promises its callers, over-allocating when that
 * is stricter than what the source guarantees, so that a block carved at an
 * aligned offset is aligned too.
 */
class memory_heap
{
public:
	/**
	 * @brief Take a region from a source.
	 *
	 * @param source The source to take the region from. Must outlive this.
	 * @param size Usable size of the heap, in bytes. Must be greater than zero.
	 * @param alignment Alignment of the heap's base, in bytes. Must be a power
	 * of two.
	 *
	 * @throws std::bad_alloc If there was not enough memory left.
	 * @throws error If the region could not be obtained for any other reason.
	 */
	memory_heap(
		memory_source &source,
		std::size_t size,
		std::size_t alignment
	);
	memory_heap(const memory_heap &other) = delete;
	memory_heap(memory_heap &&other) = delete;
	~memory_heap();

	memory_heap& operator=(const memory_heap &other) = delete;
	memory_heap& operator=(memory_heap &&other) = delete;

	/**
	 * @brief Get the start of the usable region.
	 *
	 * @return void* Pointer to the first byte. Never null, and aligned to the
	 * alignment this was constructed with.
	 */
	void* get_data() const noexcept;

	/**
	 * @brief Get the usable size of the heap.
	 *
	 * @return std::size_t Size, in bytes.
	 */
	std::size_t get_size() const noexcept;

private:
	memory_source *m_source;

	/// What the source returned, which is what has to be given back to it. Only
	/// differs from the usable base when the source aligns more loosely than
	/// the heap promises.
	void *m_allocation;

	void *m_data;
	std::size_t m_size;
};

} // namespace cuda
} // namespace rexlib
