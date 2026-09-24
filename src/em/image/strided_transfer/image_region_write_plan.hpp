// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "image_region_offsets.hpp"

#include <rexlib/core/layout/joint_layout.hpp>

namespace rexlib
{
namespace em
{

/**
 * @brief A batch of regions to be written, ready to be walked.
 *
 * A write writes the file, so the file is named first in the layout.
 * That is what settles the order the axes are walked in, and so makes the
 * traversal sequential in the file.
 *
 * That is the whole of what separates this from
 * @ref image_region_read_plan, and it is why the two are separate types
 * rather than one carrying a direction: a batch resolved for one of them
 * states nothing that would make it correct for the other, and only
 * @ref write_regions accepts this one.
 */
class image_region_write_plan
{
public:
	/**
	 * @brief Resolve a batch of regions and build the space to walk it in.
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
	image_region_write_plan(
		const image_transfer_plan &regions,
		span<const std::size_t> file_extents,
		span<const std::ptrdiff_t> file_strides,
		span<const std::size_t> array_extents,
		span<const std::ptrdiff_t> array_strides,
		std::ptrdiff_t array_offset
	);

	image_region_write_plan(const image_region_write_plan &other) = delete;
	image_region_write_plan(image_region_write_plan &&other) noexcept = default;
	~image_region_write_plan() = default;

	image_region_write_plan&
	operator=(const image_region_write_plan &other) = delete;
	image_region_write_plan&
	operator=(image_region_write_plan &&other) noexcept = default;

	/**
	 * @brief Get where each region starts on each side.
	 *
	 * @return const image_region_offsets& The offsets.
	 */
	const image_region_offsets& get_offsets() const noexcept;

	/**
	 * @brief Get the space every region is walked in.
	 *
	 * @return const joint_layout& The layout, naming the file first.
	 */
	const joint_layout& get_layout() const noexcept;

private:
	image_region_offsets m_offsets;
	joint_layout m_layout;
};

} // namespace em
} // namespace rexlib
