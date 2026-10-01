// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>
#include <rexlib/em/image/index_table.hpp>

#include <cstddef>

namespace rexlib
{
namespace em
{

/**
 * @brief The regions to transfer between one file and one array.
 *
 * Every region pairs an offset into the file with an offset into the array,
 * and every region in a plan shares one @ref image_transfer_shape. The two
 * sides are named file and array rather than source and destination, so the
 * same plan describes a read and a write alike.
 *
 * The shape is fixed when a plan is constructed; only the regions come and
 * go.
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
	 * @param shape The shape of every region.
	 */
	REXLIB_API
	explicit image_transfer_plan(image_transfer_shape shape);

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
	 * file. Its size must equal the file rank of the shape.
	 * @param array_offset Index of the first element of the region in the
	 * array. Its size must equal the array rank of the shape.
	 * @throws std::invalid_argument If either offset has the wrong rank.
	 */
	REXLIB_API
	void add(
		span<const std::size_t> file_offset,
		span<const std::size_t> array_offset
	);

	/**
	 * @brief Drop the regions held, keeping the shape and the capacity.
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
	 * @brief Get the shape every region shares.
	 *
	 * @return const image_transfer_shape& The shape. It refers to storage
	 * owned by this plan.
	 */
	REXLIB_API
	const image_transfer_shape& get_shape() const noexcept;

	/**
	 * @brief Get where a region starts in the file.
	 *
	 * @param region_index Index of the region. Must be below
	 * @ref get_region_count.
	 * @return span<const std::size_t> The offset, of the file rank of the
	 * shape. It refers to storage owned by this plan.
	 */
	REXLIB_API
	span<const std::size_t>
	get_file_offset(std::size_t region_index) const noexcept;

	/**
	 * @brief Get where a region starts in the array.
	 *
	 * @param region_index Index of the region. Must be below
	 * @ref get_region_count.
	 * @return span<const std::size_t> The offset, of the array rank of the
	 * shape. It refers to storage owned by this plan.
	 */
	REXLIB_API
	span<const std::size_t>
	get_array_offset(std::size_t region_index) const noexcept;

private:
	image_transfer_shape m_shape;
	index_table m_file_offsets;
	index_table m_array_offsets;
};

} // namespace em
} // namespace rexlib
