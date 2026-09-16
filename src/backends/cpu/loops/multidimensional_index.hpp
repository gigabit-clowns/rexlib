// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The coordinates of an element within the index space of a
 * traversal.
 *
 * A view over the offsets of the index operands in a cursor, so it is only
 * valid until that cursor moves.
 */
class multidimensional_index
{
public:
	/**
	 * @brief Construct an index that is not placed at any element yet.
	 *
	 * @param rank Number of axes of the index space.
	 * @param inner_axis Axis of the index space that the innermost axis of the
	 * traversal walks. Equal to @p rank when the traversal has no axis.
	 */
	REXLIB_CONSTEXPR
	multidimensional_index(std::size_t rank, std::size_t inner_axis) noexcept;

	/**
	 * @brief Get the number of axes of the index space.
	 *
	 * @return std::size_t The rank.
	 */
	REXLIB_CONSTEXPR std::size_t get_rank() const noexcept;

	/**
	 * @brief Get the coordinate along an axis.
	 *
	 * @param axis The axis. Must be less than `get_rank()`.
	 * @return std::size_t The coordinate.
	 */
	REXLIB_CONSTEXPR std::size_t operator[](std::size_t axis) const noexcept;

	/**
	 * @brief Get this index placed at the element a cursor points to.
	 *
	 * @param coordinates Offsets of the cursor, starting at the index operand
	 * of the first axis.
	 * @return multidimensional_index The index of that element.
	 */
	REXLIB_CONSTEXPR multidimensional_index
	rebased(const std::ptrdiff_t *coordinates) const noexcept;

	/**
	 * @brief Get this index a number of elements further along the innermost
	 * axis of the traversal.
	 *
	 * @param count Number of elements to advance by.
	 * @return multidimensional_index The index of that element.
	 */
	REXLIB_CONSTEXPR
	multidimensional_index advanced(std::size_t count) const noexcept;

private:
	REXLIB_CONSTEXPR multidimensional_index(
		const std::ptrdiff_t *coordinates,
		std::size_t rank,
		std::size_t inner_axis,
		std::size_t inner_offset
	) noexcept;

	const std::ptrdiff_t *m_coordinates;
	std::size_t m_rank;
	std::size_t m_inner_axis;
	std::size_t m_inner_offset;
};

} // namespace cpu
} // namespace rexlib

#include "multidimensional_index.inl"
