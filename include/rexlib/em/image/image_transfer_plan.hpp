// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief The regions transferred in one call.
 *
 * Every region pairs an offset into the source with an offset into the
 * destination, and every region in a plan shares one set of extents. The
 * two sides are described alike and neither is privileged; which of them is
 * the file is decided by the call the plan is handed to, since
 * @ref image_reader::read takes the file as its source and
 * @ref image_writer::write takes it as its destination.
 *
 * The extents are the shape of one region and nothing else, so they carry
 * the rank of the region rather than the rank of either side. A side of
 * higher rank spans a single position along the axes the extents do not
 * reach, which are its leading ones: a batch of two dimensional patches is
 * cut out of a two dimensional micrograph into a three dimensional
 * destination with extents of rank two, a source offset of rank two and a
 * destination offset of rank three, and neither side pads the extents.
 * Ranks may differ in either direction, so a plane of a stack may equally be
 * read into an array that does not carry the axis it was stacked along.
 *
 * The offsets are held in two flat vectors instead of one allocation per
 * region, so a batch of any size costs a bounded number of allocations, and
 * @ref clear keeps the capacity: one instance reused from one call to the
 * next allocates nothing after the first. That is the point of the class,
 * and reusing it is the expected way to use it.
 */
class image_transfer_plan
{
public:
	/**
	 * @brief Construct an empty plan holding no region and no extents.
	 */
	REXLIB_API
	image_transfer_plan() noexcept;

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
	 * @brief Set the shape shared by the regions and drop the ones held.
	 *
	 * Keeps the capacity already reserved, so configuring a reused plan for
	 * the next call allocates nothing.
	 *
	 * @param extents Extents of one region. Their rank is the rank of the
	 * region, which may be lower than that of either side.
	 * @param source_rank Rank of the array the regions are read from.
	 * @param destination_rank Rank of the array the regions are written to.
	 * @throws std::invalid_argument If the rank of @p extents exceeds
	 * @p source_rank or @p destination_rank.
	 */
	REXLIB_API
	void reset(
		span<const std::size_t> extents,
		std::size_t source_rank,
		std::size_t destination_rank
	);

	/**
	 * @brief Append one region.
	 *
	 * @param source_offset Index of the first element of the region in the
	 * source. Its size must equal @ref get_source_rank.
	 * @param destination_offset Index of the first element of the region in
	 * the destination. Its size must equal @ref get_destination_rank.
	 * @throws std::invalid_argument If either offset has the wrong rank.
	 */
	REXLIB_API
	void add(
		span<const std::size_t> source_offset,
		span<const std::size_t> destination_offset
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
	std::size_t get_size() const noexcept;

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
	 * @brief Get the rank of the array the regions are read from.
	 *
	 * @return std::size_t The rank.
	 */
	REXLIB_API
	std::size_t get_source_rank() const noexcept;

	/**
	 * @brief Get the rank of the array the regions are written to.
	 *
	 * @return std::size_t The rank.
	 */
	REXLIB_API
	std::size_t get_destination_rank() const noexcept;

	/**
	 * @brief Get the extents shared by every region.
	 *
	 * @return span<const std::size_t> The extents, of rank @ref get_rank.
	 */
	REXLIB_API
	span<const std::size_t> get_extents() const noexcept;

	/**
	 * @brief Get where a region starts in the source.
	 *
	 * @param index Index of the region. Must be below @ref get_size.
	 * @return span<const std::size_t> The offset, of rank
	 * @ref get_source_rank. It refers to storage owned by this plan.
	 */
	REXLIB_API
	span<const std::size_t>
	get_source_offset(std::size_t index) const noexcept;

	/**
	 * @brief Get where a region starts in the destination.
	 *
	 * @param index Index of the region. Must be below @ref get_size.
	 * @return span<const std::size_t> The offset, of rank
	 * @ref get_destination_rank. It refers to storage owned by this plan.
	 */
	REXLIB_API
	span<const std::size_t>
	get_destination_offset(std::size_t index) const noexcept;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::size_t> m_extents;
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::size_t> m_source_offsets;
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::size_t> m_destination_offsets;
	std::size_t m_source_rank;
	std::size_t m_destination_rank;
	std::size_t m_size;
};

/**
 * @brief Get the extent a region spans along one axis of a side.
 *
 * The extents of a plan cover the trailing axes of each side, and a side
 * spans a single position along the leading axes they do not reach. This
 * resolves either case for one axis, so that a caller walking a side does
 * not repeat the arithmetic.
 *
 * @param regions The plan the region belongs to.
 * @param rank Rank of the side being walked, one of
 * @ref image_transfer_plan::get_source_rank or
 * @ref image_transfer_plan::get_destination_rank.
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
