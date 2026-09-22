// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>

namespace rexlib
{
namespace em
{

class image_transfer_plan;

/**
 * @brief Merge the runs of neighbouring regions of a transfer plan.
 *
 * Two regions next to each other in a plan are neighbours when they differ
 * by one along the last axis the extents do not reach, on the file side and
 * on the array side alike, and start at the same place along every other
 * axis. A stack read or written a batch at a time is made of nothing else:
 * consecutive slices of a file landing in consecutive slots of an array.
 * Such a run is one hyperrectangle, and describing it as one is what lets a
 * reader walk it in a single pass and a writer lay it down in one stretch.
 *
 * Merging spans that axis rather than a single position along it, so the
 * merged regions carry one extent more than the plan did. A plan holds one
 * set of extents for every region in it, so runs of different lengths cannot
 * share one and get a plan each, in the order their length was first met.
 * The regions left alone keep the extents they had, in a plan of their own.
 *
 * Only regions next to each other in @p regions are ever merged, so a caller
 * that adds a run out of order is read or written region by region.
 *
 * @param regions The regions to merge. Must address a single file, which is
 * what @ref make_file_transfer_plan produces.
 * @param result Where the plans are written. Cleared first, and left empty
 * when false is returned.
 * @return true Some run was merged, and @p result holds what to transfer in
 * place of @p regions.
 * @return false No two regions are neighbours, so @p regions itself is what
 * to transfer.
 */
bool make_merged_transfer_plans(
	const image_transfer_plan &regions,
	std::vector<image_transfer_plan> &result
);

} // namespace em
} // namespace rexlib
