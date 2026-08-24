// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "joint_cursor.hpp"
#include "../span.hpp"
#include "../platform/dynamic_shared_object.h"

#include <memory>

namespace rex 
{

class joint_layout_implementation;

/**
 * @brief Provides joint access semantics for multiple operands
 * 
 * This class enables jointly iterating over multiple operands. It stores
 * extents of the iteration space, strides for each of the operands, 
 * and offset for each of the operands.
 * 
 * Instances of this object must be obtained through 
 * joint_layout_builder. Once built, it is inmutable.
 * 
 * @see joint_layout_builder
 * 
 */
class joint_layout
{
public:
	/**
	 * Constant value to be passed to the `last_dim` parameter of `iter()` and
	 * `next()` to signal the past-the-end axis of the layout.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t end_dim = 
		std::numeric_limits<std::size_t>::max();

	REXLIB_API joint_layout();
	REXLIB_API explicit joint_layout(
		std::unique_ptr<
			const joint_layout_implementation
		> implementation
	);
	joint_layout(const joint_layout&) = delete;
	REXLIB_API 
	joint_layout(joint_layout&& other) noexcept;
	REXLIB_API ~joint_layout();

	joint_layout& 
	operator=(const joint_layout&) = delete;
	REXLIB_API joint_layout& 
	operator=(joint_layout&& other) noexcept;

	/**
	 * @brief Get the number of operands
	 * 
	 * @return std::size_t The number of operands.
	 */
	REXLIB_API std::size_t get_number_of_operands() const noexcept;

	/**
	 * @brief Get the number of dimensions.
	 * 
	 * @return std::size_t Number of dimensions.
	 */
	REXLIB_API std::size_t get_rank() const noexcept;

	/**
	 * @brief Get the extents of the iteration space.
	 *
	 * @return span<const std::size_t> The extents of the iteration space.
	 */
	REXLIB_API span<const std::size_t> get_extents() const;

	/**
	 * @brief Computes the number of positions the iteration space holds.
	 *
	 * The product of the extents, which is how many times a traversal of the
	 * whole layout visits its operands, and so the range `seek()` indexes
	 * into. A layout of no axes holds a single position, the identity of 
	 * the product.
	 *
	 * @return std::size_t Number of positions. 0 if the layout can not be
	 * iterated, as `iter()` and `seek()` report it.
	 */
	REXLIB_API std::size_t compute_element_count() const noexcept;

	/**
	 * @brief Get the strides for an operand.
	 * 
	 * @param operand The index of the operand. Must be in 
	 * [0, number_of_operands())
	 * @return span<const std::ptrdiff_t> The strides of the requested operand. 
	 * Always has the same size as extents.
	 * 
	 * @throws std::out_of_range if operand index is out of range.
	 */
	REXLIB_API
	span<const std::ptrdiff_t> get_strides(std::size_t operand) const;

	/**
	 * @brief Get the offset parameter of an operand.
	 * 
	 * @param operand The index of the operand. Must be in 
	 * [0, number_of_operands())
	 * @return std::ptrdiff_t The offset parameter of the requested operand.
	 * 
	 * @throws std::out_of_range if operand index is out of range.
	 */
	REXLIB_API
	std::ptrdiff_t get_offset(std::size_t operand) const;

	/**
	 * @brief Populate a cursor for traversing this layout.
	 * 
	 * @param ite The iterator to be populated.
	 * @param first_dim Index of the inner-most dimension to be iterated. Must
	 * be less or equal to get_rank(). Defaults to zero.
	 * @param last_dim Index of the outer-most dimension to be iterated. Must
	 * be less or equal to `get_rank()` or equal to `end_dim`. Defaults to 
	 * `end_dim`.
	 * @return std::size_t Number of elements in the dimension. 0 if the
	 * layout can not be iterated.
	 */
	REXLIB_API
	std::size_t iter(
		joint_cursor &ite, 
		std::size_t first_dim = 0,
		std::size_t last_dim = end_dim
	) const;

	/**
	 * @brief Populate a cursor for traversing this layout, positioned at a
	 * given point of the traversal.
	 *
	 * The position counts the way the traversal does, `first_dim` varying
	 * fastest, so that a range of it names a contiguous stretch of the very
	 * iteration `next()` performs: two cursors splitting that range visit
	 * between them exactly what one cursor visits alone. This is what lets an
	 * iteration be shared out, whether over threads or over anything else.
	 *
	 * `seek(ite, 0, ...)` is `iter(ite, ...)`.
	 *
	 * @param ite The iterator to be populated.
	 * @param position Index into the iterated range, counting `first_dim`
	 * fastest. Must be less than the number of positions that range holds.
	 * Otherwise behavior is undefined.
	 * @param first_dim Index of the inner-most dimension to be iterated. Must
	 * be less or equal to get_rank(). Defaults to zero.
	 * @param last_dim Index of the outer-most dimension to be iterated. Must
	 * be less or equal to `get_rank()` or equal to `end_dim`. Defaults to
	 * `end_dim`.
	 * @return std::size_t Number of remaining elements in the inner-most
	 * iterated dimension, as `iter()` reports it. 0 if the layout can not be
	 * iterated.
	 */
	REXLIB_API
	std::size_t seek(
		joint_cursor &ite,
		std::size_t position,
		std::size_t first_dim = 0,
		std::size_t last_dim = end_dim
	) const;

	/**
	 * @brief Advance a cursor.
	 *
	 * @param ite The iterator to be advanced. Must have been populated by
	 * `iter()`. The previous call to `iter()` or `next()` must have returned
	 * a non-zero value.
	 * @param n Number of elements to be advanced. Must be less or equal to the 
	 * number returned by the previous call to `iter()` or `next()`. Otherwise 
	 * behavior is undefined.
	 * @param first_dim Index of the inner-most dimension to be iterated. Must
	 * be less or equal to `get_rank()`. Defaults to zero. 
	 * @param last_dim Index of the outer-most dimension to be iterated. Must
	 * be less or equal to `get_rank()` or equal to `end_dim`. Defaults to 
	 * `end_dim`.
	 * @return std::size_t Number of remaining elements in `dim`. 0 if iteration 
	 * has finished.
	 * 
	 * @warning If `first_dim` or `last_dim` is modified during iteration, the 
	 * value provided to `n` needs special attention as the limit may be smaller 
	 * than the last value returned by `iter()` or last `next()`. If unsure, 
	 * the new limit may be computed using `next(ite, 0, first_dim, last_dim)`.
	 */
	REXLIB_API
	std::size_t next(
		joint_cursor &ite, 
		std::size_t n, 
		std::size_t first_dim = 0,
		std::size_t last_dim = end_dim

	) const noexcept;

	/**
	 * @brief Get a pointer to the implementation.
	 * 
	 * @return const joint_layout_implementation* Pointer to the
	 * implementation.
	 *
	 * @note This method is meant to be used for testing purposes, as the
	 * joint_layout_implementation is not publicly available.
	 */
	const joint_layout_implementation* 
	get_implementation() const noexcept;

private:
	std::unique_ptr<
		const joint_layout_implementation
	> m_implementation;
};

} // namespace rex
