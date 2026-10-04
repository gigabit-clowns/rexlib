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
 * @brief The shape every region of a transfer shares: the extents of one
 * region and the ranks of the file and the array it joins.
 *
 * The extents are the shape of one region and nothing else, so they carry
 * the rank of the region rather than the rank of either side. A side of
 * higher rank spans a single position along the axes the extents do not
 * reach, which are its implicit leading ones, and neither side pads the
 * extents. For example, two dimensional regions of a two dimensional file
 * placed side by side along the first axis of a three dimensional array have
 * extents of rank two, a file rank of two and an array rank of three. Ranks may
 * differ in either direction, so a plane of a three dimensional file may
 * equally go to a two dimensional array.
 *
 * @see image_transfer_plan
 * @see image_transaction_plan
 */
class image_transfer_shape
{
public:
	/**
	 * @brief Construct a shape from its components.
	 *
	 * @param extents Extents of one region. Their rank is the rank of the
	 * region, which may be lower than that of either side.
	 * @param file_rank Rank of the file side.
	 * @param array_rank Rank of the array side.
	 * @throws std::invalid_argument If the rank of @p extents exceeds
	 * @p file_rank or @p array_rank.
	 */
	REXLIB_API
	image_transfer_shape(
		std::vector<std::size_t> extents,
		std::size_t file_rank,
		std::size_t array_rank
	);

	REXLIB_API
	image_transfer_shape(const image_transfer_shape &other);
	REXLIB_API
	image_transfer_shape(image_transfer_shape &&other) noexcept;
	REXLIB_API
	~image_transfer_shape();

	REXLIB_API
	image_transfer_shape& operator=(const image_transfer_shape &other);
	REXLIB_API
	image_transfer_shape& operator=(image_transfer_shape &&other) noexcept;

	/**
	 * @brief Get the extents of one region.
	 *
	 * @return span<const std::size_t> The extents, of rank @ref get_rank. It
	 * refers to storage owned by this shape.
	 */
	REXLIB_API
	span<const std::size_t> get_extents() const noexcept;

	/**
	 * @brief Get the rank of one region.
	 *
	 * @return std::size_t The rank of the extents.
	 */
	REXLIB_API
	std::size_t get_rank() const noexcept;

	/**
	 * @brief Get the rank of the file side.
	 *
	 * @return std::size_t The rank, never below @ref get_rank.
	 */
	REXLIB_API
	std::size_t get_file_rank() const noexcept;

	/**
	 * @brief Get the rank of the array side.
	 *
	 * @return std::size_t The rank, never below @ref get_rank.
	 */
	REXLIB_API
	std::size_t get_array_rank() const noexcept;

	/**
	 * @brief Get how many leading axes of a side a region spans a single
	 * position along.
	 *
	 * @param rank Rank of the side. Must not be below @ref get_rank.
	 * @return std::size_t The number of axes the extents do not reach.
	 */
	REXLIB_API
	std::size_t get_leading_rank(std::size_t rank) const noexcept;

	/**
	 * @brief Get the extent a region spans along one axis of a side.
	 *
	 * @param rank Rank of the side. Must not be below @ref get_rank.
	 * @param axis Index of the axis, below @p rank.
	 * @return std::size_t One along a leading axis, and the extent the axis
	 * corresponds to otherwise.
	 */
	REXLIB_API
	std::size_t get_extent(std::size_t rank, std::size_t axis) const noexcept;

private:
	std::vector<std::size_t> m_extents;
	std::size_t m_file_rank;
	std::size_t m_array_rank;
};

} // namespace em
} // namespace rexlib
