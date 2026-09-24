// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

class image_transfer_plan;

/**
 * @brief Where each region of a batch starts on each of the two sides.
 *
 * Every region of a batch has the same extents and differs only in where it
 * starts, so all of them are walked by one iteration space and only the two
 * base pointers move. This is that pair of pointer offsets, one per region.
 *
 * The pairs are held in ascending file order rather than in the order the
 * batch states them, so that a transfer walks the file forwards and the
 * regions it is about to touch form runs that can be asked for together. The
 * index of a pair is therefore its position in that order, not the one it was
 * added at.
 *
 * Resolving them is also where every region is bounds checked, so a batch
 * that does not fit is refused here, before anything has been moved, rather
 * than halfway through it.
 *
 * The extents and the strides the offsets were resolved from are not kept:
 * a @ref joint_layout is what they become, and holding them here as well
 * would be a second copy of what it already carries.
 */
class image_region_offsets
{
public:
	/**
	 * @brief Resolve a batch of regions against the two sides they address.
	 *
	 * The extents of @p regions cover the trailing axes of each side, which
	 * spans a single position along the leading axes they do not reach.
	 *
	 * @param regions The regions to move.
	 * @param file_extents Extents of the file.
	 * @param file_strides Distance between consecutive elements of the file
	 * along each axis, in elements.
	 * @param array_extents Extents of the array.
	 * @param array_strides Distance between consecutive elements of the
	 * array along each axis, in elements.
	 * @param array_offset Index of the first element of the array.
	 * @throws std::invalid_argument If a rank does not match the batch or
	 * the strides do not match their extents.
	 * @throws std::out_of_range If a region is not contained in the file or
	 * in the array where it is placed.
	 */
	image_region_offsets(
		const image_transfer_plan &regions,
		span<const std::size_t> file_extents,
		span<const std::ptrdiff_t> file_strides,
		span<const std::size_t> array_extents,
		span<const std::ptrdiff_t> array_strides,
		std::ptrdiff_t array_offset
	);

	image_region_offsets(const image_region_offsets &other) = default;
	image_region_offsets(image_region_offsets &&other) noexcept = default;
	~image_region_offsets() = default;

	image_region_offsets&
	operator=(const image_region_offsets &other) = default;
	image_region_offsets&
	operator=(image_region_offsets &&other) noexcept = default;

	/**
	 * @brief Get how many regions the batch holds.
	 *
	 * @return std::size_t The number of regions.
	 */
	std::size_t get_region_count() const noexcept;

	/**
	 * @brief Get where each region starts in the array, in elements.
	 *
	 * @return span<const std::ptrdiff_t> One offset per region, ordered by
	 * the file offset it is paired with.
	 */
	span<const std::ptrdiff_t> get_array() const noexcept;

	/**
	 * @brief Get where each region starts in the file, in elements.
	 *
	 * @return span<const std::ptrdiff_t> One offset per region, ascending.
	 */
	span<const std::ptrdiff_t> get_file() const noexcept;

private:
	std::vector<std::ptrdiff_t> m_array;
	std::vector<std::ptrdiff_t> m_file;
};

} // namespace em
} // namespace rexlib
