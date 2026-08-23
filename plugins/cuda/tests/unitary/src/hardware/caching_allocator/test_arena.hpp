// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace xmipp4
{
namespace cuda
{

/**
 * @brief Ordinary host memory standing in for a device's.
 *
 * What a mocked @ref memory_source hands out, so that the machinery above it
 * can be exercised on a machine with no CUDA device. Tests can say how much
 * memory there is, and ask what is still outstanding.
 */
class test_arena
{
public:
	/// Stands for an arena that never runs out.
	static constexpr std::size_t unlimited =
		std::numeric_limits<std::size_t>::max();

	/**
	 * @brief Construct an arena.
	 *
	 * @param capacity Total number of bytes that can be outstanding at once.
	 * @param alignment Alignment every region is aligned to.
	 */
	explicit test_arena(
		std::size_t capacity = unlimited,
		std::size_t alignment = 256
	) noexcept;
	test_arena(const test_arena &other) = delete;
	test_arena(test_arena &&other) = delete;
	~test_arena();

	test_arena& operator=(const test_arena &other) = delete;
	test_arena& operator=(test_arena &&other) = delete;

	/**
	 * @brief Take a region out of the arena.
	 *
	 * @param size Size of the region, in bytes.
	 * @return void* Pointer to the region.
	 *
	 * @throws std::bad_alloc If the arena has that much less than that left.
	 */
	void* take(std::size_t size);

	/**
	 * @brief Give a region back to the arena.
	 *
	 * @param data Pointer returned by @ref take. Anything else is ignored.
	 */
	void give_back(void *data) noexcept;

	/**
	 * @brief Get the alignment every region is aligned to.
	 *
	 * @return std::size_t Alignment, in bytes.
	 */
	std::size_t get_alignment() const noexcept;

	/**
	 * @brief Get how many regions are currently outstanding.
	 *
	 * @return std::size_t Number of regions.
	 */
	std::size_t get_region_count() const noexcept;

	/**
	 * @brief Get how many bytes are currently outstanding.
	 *
	 * @return std::size_t Number of bytes.
	 */
	std::size_t get_used_bytes() const noexcept;

private:
	using region_vector = std::vector<std::pair<void*, std::size_t>>;

	std::size_t m_capacity;
	std::size_t m_alignment;
	std::size_t m_used_bytes;
	region_vector m_regions;

	void release(void *data) noexcept;
};

} // namespace cuda
} // namespace xmipp4
