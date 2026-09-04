// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstddef>

namespace rexlib
{
namespace em
{

class image_transaction_plan;
class region_grouping;

/**
 * @brief Count the files a grouping actually holds a region for.
 *
 * A file named in a transaction plan but never given a region is still
 * one of @ref region_grouping::get_file_count, with zero regions; this
 * counts only the ones worth dispatching a file-level unit of work for.
 *
 * @param grouping The grouping to count over.
 * @return std::size_t The number of files with at least one region.
 */
std::size_t count_files_with_regions(const region_grouping &grouping);

/**
 * @brief Build the transfer plan for one file of a grouped transaction.
 *
 * Copies the shape from @p plan and appends every region @p grouping
 * holds for @p file_index, in the order the grouping holds them.
 *
 * @param plan The transaction the shape and the regions come from.
 * @param grouping The grouping naming which of the plan's regions
 * belong to @p file_index.
 * @param file_index Index of the file. Must be below
 * @ref region_grouping::get_file_count and hold at least one region.
 * @return image_transfer_plan The transfer plan for that file alone.
 */
image_transfer_plan build_file_transfer_plan(
	const image_transaction_plan &plan,
	const region_grouping &grouping,
	std::size_t file_index
);

} // namespace em
} // namespace rexlib
