// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstddef>

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief Check that every region of a plan fits the file and the array it
 * joins.
 *
 * A TIFF file is moved a page at a time, so regions checked only as their
 * page comes up would leave the pages before a bad one already moved. This
 * checks all of them against the whole file beforehand.
 *
 * @param regions The regions to check.
 * @param file What the file holds.
 * @param array_extents Extents of the array.
 * @param array_strides Distance between consecutive elements of the array
 * along each axis, in elements.
 * @param array_offset Index of the first element of the array.
 * @throws std::invalid_argument If a rank does not match the plan or the
 * strides do not match their extents.
 * @throws std::out_of_range If a region is not contained in the file or in
 * the array where it is placed.
 */
void check_region_bounds(
	const image_transfer_plan &regions,
	const image_descriptor &file,
	span<const std::size_t> array_extents,
	span<const std::ptrdiff_t> array_strides,
	std::ptrdiff_t array_offset
);

} // namespace tiff
} // namespace em
} // namespace rexlib
