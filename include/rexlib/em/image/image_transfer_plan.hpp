// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/index_table.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief The regions to transfer between one file and one array.
 *
 * Every region pairs an offset into the file with an offset into the array,
 * and every region in a plan shares one set of extents. The two sides are
 * named file and array rather than source and destination, so the same plan
 * describes a read and a write alike.
 *
 * The extents are the shape of one region and nothing else, so they carry
 * the rank of the region rather than the rank of either side. A side of
 * higher rank spans a single position along the axes the extents do not
 * reach, which are its leading ones, and neither side pads the extents. For
 * example, two dimensional regions of a two dimensional file placed side by
 * side along the first axis of a three dimensional array have extents of
 * rank two, file offsets of rank two and array offsets of rank three. Ranks
 * may differ in either direction, so a plane of a three dimensional file
 * may equally go to a two dimensional array.
 *
 * The shape, which is the extents and the two ranks, is fixed when a plan is
 * constructed; only the regions come and go.
 *
 * The offsets are held in two @ref index_table rather than one allocation
 * per region, so any number of regions costs a bounded number of
 * allocations, and @ref clear keeps the capacity, so a plan refilled after
 * its first use allocates nothing.
 *
 * @see image_transaction_plan
 */
class image_transfer_plan
{
public:
	/**
	 * @brief Construct an empty plan of a given shape.
	 *
	 * The shape is what every region of the plan shares and is fixed for
	 * the life of it; only the regions are added and dropped.
	 *
	 * @param extents Extents of one region. Their rank is the rank of the
	 * region, which may be lower than that of either side.
	 * @param file_rank Rank of the file the regions address.
	 * @param array_rank Rank of the array the regions address.
	 * @throws std::invalid_argument If the rank of @p extents exceeds
	 * @p file_rank or @p array_rank.
	 */
	REXLIB_API
	image_transfer_plan(
		span<const std::size_t> extents,
		std::size_t file_rank,
		std::size_t array_rank
	);

	REXLIB_API
	image_transfer_plan(const image_transfer_plan &other);
	REXLIB_API
	image_transfer_plan(image_transfer_plan &&other) noexcept;
	REXLIB_API
	~image_transfer_plan();

	REXLIB_API
	image_transfer_plan& operator=(const image_transfer_plan &other);
	REXLIB_API
	image_transfer_plan& operator=(image_transfer_plan &&other) noexcept;

	/**
	 * @brief Append one region.
	 *
	 * @param file_offset Index of the first element of the region in the
	 * file. Its size must equal @ref get_file_rank.
	 * @param array_offset Index of the first element of the region in the
	 * array. Its size must equal @ref get_array_rank.
	 * @throws std::invalid_argument If either offset has the wrong rank.
	 */
	REXLIB_API
	void add(
		span<const std::size_t> file_offset,
		span<const std::size_t> array_offset
	);

	/**
	 * @brief Drop the regions held, keeping the extents and the capacity.
	 */
	REXLIB_API
	void clear() noexcept;

	/**
	 * @brief Make room for a number of regions without allocating later.
	 *
	 * @param count Number of regions to make room for.
	 */
	REXLIB_API
	void reserve(std::size_t count);

	/**
	 * @brief Get how many regions are held.
	 *
	 * @return std::size_t The number of regions.
	 */
	REXLIB_API
	std::size_t get_region_count() const noexcept;

	/**
	 * @brief Get the rank of one region.
	 *
	 * The rank of the extents. Each side spans a single position along the
	 * axes beyond it.
	 *
	 * @return std::size_t The rank.
	 */
	REXLIB_API
	std::size_t get_rank() const noexcept;

	/**
	 * @brief Get the rank of the file the regions address.
	 *
	 * @return std::size_t The rank.
	 */
	REXLIB_API
	std::size_t get_file_rank() const noexcept;

	/**
	 * @brief Get the rank of the array the regions address.
	 *
	 * @return std::size_t The rank.
	 */
	REXLIB_API
	std::size_t get_array_rank() const noexcept;

	/**
	 * @brief Get the extents shared by every region.
	 *
	 * @return span<const std::size_t> The extents, of rank @ref get_rank.
	 */
	REXLIB_API
	span<const std::size_t> get_extents() const noexcept;

	/**
	 * @brief Get where a region starts in the file.
	 *
	 * @param region_index Index of the region. Must be below
	 * @ref get_region_count.
	 * @return span<const std::size_t> The offset, of rank
	 * @ref get_file_rank. It refers to storage owned by this plan.
	 */
	REXLIB_API
	span<const std::size_t>
	get_file_offset(std::size_t region_index) const noexcept;

	/**
	 * @brief Get where a region starts in the array.
	 *
	 * @param region_index Index of the region. Must be below
	 * @ref get_region_count.
	 * @return span<const std::size_t> The offset, of rank
	 * @ref get_array_rank. It refers to storage owned by this plan.
	 */
	REXLIB_API
	span<const std::size_t>
	get_array_offset(std::size_t region_index) const noexcept;

private:
	std::vector<std::size_t> m_extents;
	index_table m_file_offsets;
	index_table m_array_offsets;
};

/**
 * @brief Get the extent a region spans along one axis of a side.
 *
 * The extents of a plan cover the trailing axes of each side, and a side
 * spans a single position along the leading axes they do not reach. This
 * resolves either case for one axis.
 *
 * @param regions The plan the region belongs to.
 * @param rank Rank of the side, one of
 * @ref image_transfer_plan::get_file_rank or
 * @ref image_transfer_plan::get_array_rank.
 * @param axis Index of the axis, below @p rank.
 * @return std::size_t The extent along @p axis.
 */
REXLIB_API
std::size_t get_region_extent(
	const image_transfer_plan &regions,
	std::size_t rank,
	std::size_t axis
) noexcept;

} // namespace em
} // namespace rexlib
