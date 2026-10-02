// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

class image_transaction_plan;
class image_transfer_plan;

/**
 * @brief Orders the regions of a transaction plan by the file they
 * address.
 *
 * Every region of one file ends up together, and regions of one file keep
 * the order they have in the plan.
 *
 * It does not hold the regions, only where they are: @ref get_region maps a
 * position of the ordering to a region of the plan it was built from, which
 * the plan is then asked about. One file is walked with
 *
 * @code
 * const auto first = grouping.get_first_position(file);
 * const auto count = grouping.get_file_region_count(file);
 * for (auto i = first; i < first + count; ++i)
 * {
 *     const auto region = grouping.get_region(i);
 *     plan.get_file_offset(region);
 * }
 * @endcode
 *
 * @ref build costs one pass over the regions rather than a comparison sort,
 * and keeps the capacity, so a grouping rebuilt after its first use
 * allocates nothing.
 */
class image_region_grouping
{
public:
	/**
	 * @brief Construct a grouping over no region and no file.
	 */
	image_region_grouping() noexcept;

	image_region_grouping(const image_region_grouping &other);
	image_region_grouping(image_region_grouping &&other) noexcept;
	~image_region_grouping();

	image_region_grouping& operator=(const image_region_grouping &other);
	image_region_grouping& operator=(image_region_grouping &&other) noexcept;

	/**
	 * @brief Order the regions of a plan by the file they address.
	 *
	 * Regions addressing one file keep the order they have in @p plan.
	 * Replaces whatever this grouping held, keeping its capacity.
	 *
	 * @param plan The plan whose regions are ordered.
	 */
	void build(const image_transaction_plan &plan);

	/**
	 * @brief Drop the ordering, keeping the capacity.
	 */
	void clear() noexcept;

	/**
	 * @brief Make room without allocating later.
	 *
	 * @param files Number of files to make room for.
	 * @param regions Number of regions to make room for.
	 */
	void reserve(std::size_t files, std::size_t regions);

	/**
	 * @brief Get how many regions are ordered.
	 *
	 * @return std::size_t The number of regions.
	 */
	std::size_t get_region_count() const noexcept;

	/**
	 * @brief Get how many files the regions were grouped over.
	 *
	 * @return std::size_t The number of files.
	 */
	std::size_t get_file_count() const noexcept;

	/**
	 * @brief Get how many files at least one region addresses.
	 *
	 * A file the plan named but that no region addresses counts in
	 * @ref get_file_count and not here.
	 *
	 * @return std::size_t The number of files addressed.
	 */
	std::size_t get_addressed_file_count() const noexcept;

	/**
	 * @brief Get where the regions of one file start.
	 *
	 * @param file_index Index of the file. Must be below
	 * @ref get_file_count.
	 * @return std::size_t Position of its first region in this ordering.
	 */
	std::size_t get_first_position(std::size_t file_index) const noexcept;

	/**
	 * @brief Get how many regions address one file.
	 *
	 * Zero for a file that was named but that no region addresses.
	 *
	 * @param file_index Index of the file. Must be below
	 * @ref get_file_count.
	 * @return std::size_t The number of regions.
	 */
	std::size_t get_file_region_count(std::size_t file_index) const noexcept;

	/**
	 * @brief Get the region at one position of this ordering.
	 *
	 * @param position Position in the ordering. Must be below
	 * @ref get_region_count.
	 * @return std::size_t Index of the region in the plan this was built
	 * from.
	 */
	std::size_t get_region(std::size_t position) const noexcept;

private:
	std::vector<std::size_t> m_regions;
	std::vector<std::size_t> m_first_position;
	std::vector<std::size_t> m_cursors;
};

/**
 * @brief Make the transfer plan for the regions of one file.
 *
 * Takes the shape from @p plan and appends every region @p grouping holds
 * for @p file_index, in the order it holds them.
 *
 * @param grouping The grouping the regions are read from.
 * @param plan The plan the shape and the regions come from. Must be
 * the one @p grouping was built from.
 * @param file_index Index of the file. Must be below
 * @ref image_region_grouping::get_file_count.
 * @return image_transfer_plan The transfer plan for that file alone. Empty
 * for a file no region addresses.
 */
image_transfer_plan make_file_transfer_plan(
	const image_region_grouping &grouping,
	const image_transaction_plan &plan,
	std::size_t file_index
);

} // namespace em
} // namespace rexlib
