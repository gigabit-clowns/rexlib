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
 * @brief Clip the regions of a transfer plan to both of its sides.
 *
 * A region transfers the intersection of what the file holds at its file
 * offset with what the array holds at its array offset, up to the extents of
 * the plan. The intersection begins where the region begins on either side,
 * so clipping only ever shortens the extents and never moves an offset.
 *
 * A plan carries one set of extents for every region it holds, so regions
 * clipping to different shapes cannot share one. They are grouped by the
 * shape they clip to and one plan is built per group, in the order the groups
 * were first met. A region clipping to nothing along any axis joins no group.
 *
 * @param regions The regions to clip.
 * @param file_extents Extents of the file. Must have the file rank of
 * @p regions.
 * @param array_extents Extents of the array. Must have the array rank of
 * @p regions.
 * @param result Where the plans are written. Cleared first, and left empty
 * when false is returned.
 * @return true One or more regions were clipped or dropped, and @p result
 * holds what to transfer in their place.
 * @return false Every region fits both sides whole, so @p regions itself is
 * what to transfer.
 * @throws std::invalid_argument If either extents do not have the rank of the
 * side they describe.
 */
bool make_clipped_transfer_plans(
	const image_transfer_plan &regions,
	span<const std::size_t> file_extents,
	span<const std::size_t> array_extents,
	std::vector<image_transfer_plan> &result
);

} // namespace em
} // namespace rexlib
