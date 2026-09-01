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
 * @brief The regions transferred between a dataset and an array in one call.
 *
 * Every region pairs an offset into the dataset with an offset into the
 * array, and every region in a batch shares one set of extents. Which side is
 * read and which is written is decided by the call the batch is handed to:
 * @ref image_reader::read fills the array from the dataset and
 * @ref image_writer::write does the reverse. There is one class rather than
 * two because a transfer is described by the same numbers in both
 * directions.
 *
 * The offsets are held in two flat vectors instead of one allocation per
 * region, so a batch of any size costs a bounded number of allocations, and
 * @ref clear keeps the capacity: one instance reused from one call to the
 * next allocates nothing after the first. That is the point of the class,
 * and reusing it is the expected way to use it.
 *
 * The extents are stated in the rank of the array. The dataset may have a
 * lower rank, in which case its region is the last @ref get_dataset_rank
 * extents and the ones before them must all be one; that is what lets a
 * batch of two dimensional patches be cut out of a two dimensional
 * micrograph into a three dimensional destination without a special case.
 */
class image_region_batch
{
public:
	/**
	 * @brief Construct an empty batch holding no region and no extents.
	 */
	REXLIB_API
	image_region_batch() noexcept;

	REXLIB_API
	image_region_batch(const image_region_batch &other);
	REXLIB_API
	image_region_batch(image_region_batch &&other) noexcept;
	REXLIB_API
	~image_region_batch();

	REXLIB_API
	image_region_batch& operator=(const image_region_batch &other);
	REXLIB_API
	image_region_batch& operator=(image_region_batch &&other) noexcept;

	/**
	 * @brief Set the shape shared by the regions and drop the ones held.
	 *
	 * Keeps the capacity already reserved, so configuring a reused batch for
	 * the next call allocates nothing.
	 *
	 * @param extents Extents of one region, in the rank of the array.
	 * @param dataset_rank Rank of the dataset the regions address.
	 * @throws std::invalid_argument If @p dataset_rank exceeds the rank of
	 * @p extents, or if the extents preceding the last @p dataset_rank of
	 * them are not all one.
	 */
	REXLIB_API
	void reset(span<const std::size_t> extents, std::size_t dataset_rank);

	/**
	 * @brief Append one region.
	 *
	 * @param dataset_offset Index of the first element of the region in the
	 * dataset. Its size must equal @ref get_dataset_rank.
	 * @param array_offset Index of the first element of the region in the
	 * array. Its size must equal @ref get_array_rank.
	 * @throws std::invalid_argument If either offset has the wrong rank.
	 */
	REXLIB_API
	void add(
		span<const std::size_t> dataset_offset,
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
	std::size_t get_size() const noexcept;

	/**
	 * @brief Get the rank of the dataset the regions address.
	 *
	 * @return std::size_t The rank.
	 */
	REXLIB_API
	std::size_t get_dataset_rank() const noexcept;

	/**
	 * @brief Get the rank of the array the regions address.
	 *
	 * @return std::size_t The rank, which is the rank of the extents.
	 */
	REXLIB_API
	std::size_t get_array_rank() const noexcept;

	/**
	 * @brief Get the extents shared by every region.
	 *
	 * @return span<const std::size_t> The extents, in the rank of the array.
	 */
	REXLIB_API
	span<const std::size_t> get_extents() const noexcept;

	/**
	 * @brief Get where a region starts in the dataset.
	 *
	 * @param index Index of the region. Must be below @ref get_size.
	 * @return span<const std::size_t> The offset, of rank
	 * @ref get_dataset_rank. It refers to storage owned by this batch.
	 */
	REXLIB_API
	span<const std::size_t>
	get_dataset_offset(std::size_t index) const noexcept;

	/**
	 * @brief Get where a region starts in the array.
	 *
	 * @param index Index of the region. Must be below @ref get_size.
	 * @return span<const std::size_t> The offset, of rank
	 * @ref get_array_rank. It refers to storage owned by this batch.
	 */
	REXLIB_API
	span<const std::size_t>
	get_array_offset(std::size_t index) const noexcept;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::size_t> m_extents;
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::size_t> m_dataset_offsets;
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::size_t> m_array_offsets;
	std::size_t m_dataset_rank;
	std::size_t m_size;
};

} // namespace em
} // namespace rexlib
