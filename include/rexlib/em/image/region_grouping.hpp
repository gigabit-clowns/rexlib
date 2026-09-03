// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

class image_transaction_plan;

/**
 * @brief The regions of a transaction, ordered by the file they address.
 *
 * An @ref image_transaction_plan holds its regions in the order they were
 * added, which for a batch drawn at random is an order that hops from stack
 * to stack. A consumer wants the opposite: every region of one file
 * together, so that the file is opened once and read once. This is that
 * ordering, and it is a type of its own rather than a phase of the plan,
 * because it is what a reader wants of a transaction and not something the
 * transaction is.
 *
 * It does not hold the regions, only where they are: @ref get_region maps a
 * position of the ordering to a region of the plan it was built from, which
 * the plan is then asked about. So a consumer walks one file with
 *
 * @code
 * const auto first = grouping.get_first_position(file);
 * const auto count = grouping.get_region_count(file);
 * for (auto i = first; i < first + count; ++i)
 * {
 *     const auto region = grouping.get_region(i);
 *     plan.get_file_offset(region);
 * }
 * @endcode
 *
 * @ref build costs one pass over the regions rather than a comparison sort,
 * and keeps the capacity, so one instance reused from one transaction to the
 * next allocates nothing after the first.
 */
class region_grouping
{
public:
	/**
	 * @brief Construct a grouping over no region and no file.
	 */
	REXLIB_API
	region_grouping() noexcept;

	REXLIB_API
	region_grouping(const region_grouping &other);
	REXLIB_API
	region_grouping(region_grouping &&other) noexcept;
	REXLIB_API
	~region_grouping();

	REXLIB_API
	region_grouping& operator=(const region_grouping &other);
	REXLIB_API
	region_grouping& operator=(region_grouping &&other) noexcept;

	/**
	 * @brief Order the regions of a plan by the file they address.
	 *
	 * Regions addressing one file keep the order they have in @p plan.
	 * Replaces whatever this grouping held, keeping its capacity.
	 *
	 * The grouping describes @p plan as it was when this was called. Adding
	 * to the plan afterwards does not update it, and using the two together
	 * then addresses regions that have moved; build again instead.
	 *
	 * @param plan The transaction whose regions are ordered.
	 */
	REXLIB_API
	void build(const image_transaction_plan &plan);

	/**
	 * @brief Drop the ordering, keeping the capacity.
	 */
	REXLIB_API
	void clear() noexcept;

	/**
	 * @brief Make room without allocating later.
	 *
	 * @param files Number of files to make room for.
	 * @param regions Number of regions to make room for.
	 */
	REXLIB_API
	void reserve(std::size_t files, std::size_t regions);

	/**
	 * @brief Get how many regions are ordered.
	 *
	 * @return std::size_t The number of regions.
	 */
	REXLIB_API
	std::size_t get_size() const noexcept;

	/**
	 * @brief Get how many files the regions were grouped over.
	 *
	 * @return std::size_t The number of files.
	 */
	REXLIB_API
	std::size_t get_file_count() const noexcept;

	/**
	 * @brief Get where the regions of one file start.
	 *
	 * @param file_index Index of the file. Must be below
	 * @ref get_file_count.
	 * @return std::size_t Position of its first region in this ordering.
	 */
	REXLIB_API
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
	REXLIB_API
	std::size_t get_region_count(std::size_t file_index) const noexcept;

	/**
	 * @brief Get the region at one position of this ordering.
	 *
	 * @param position Position in the ordering. Must be below
	 * @ref get_size.
	 * @return std::size_t Index of the region in the plan this was built
	 * from.
	 */
	REXLIB_API
	std::size_t get_region(std::size_t position) const noexcept;

private:
	std::vector<std::size_t> m_regions;
	std::vector<std::size_t> m_first_position;
	std::vector<std::size_t> m_cursors;
};

} // namespace em
} // namespace rexlib
