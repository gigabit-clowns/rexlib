// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "dynamic_subscript.hpp"
#include "../numerical/numerical_type.hpp"
#include "../span.hpp"
#include "../platform/attributes.hpp"
#include "../platform/dynamic_shared_object.h"

#include <memory>
#include <functional>
#include <cstddef>

namespace rexlib 
{

class strided_layout_implementation;

class strided_layout
{
public:
	REXLIB_API strided_layout() noexcept;
	REXLIB_API strided_layout(const strided_layout &other) noexcept;
	REXLIB_API strided_layout(strided_layout &&other) noexcept;
	REXLIB_API ~strided_layout();

	REXLIB_API
	strided_layout& operator=(const strided_layout &other) noexcept;
	REXLIB_API
	strided_layout& operator=(strided_layout &&other) noexcept;


	/**
	 * @brief Obtain a hash of the layout.
	 * 
	 * @return std::size_t The hash. 
	 */
	REXLIB_API
	std::size_t hash() const noexcept;

	/**
	 * @brief Get the number of axis in the layout
	 * 
	 * @return std::size_t 
	 */
	REXLIB_API
	std::size_t get_rank() const noexcept;

	/**
	 * @brief Get the extents of the layout.
	 * 
	 * @param extents Output parameter where the extents are written.
	 */
	REXLIB_API
	void get_extents(std::vector<std::size_t> &extents) const;

	/**
	 * @brief Get the strides of the layout.
	 * 
	 * @param extents Output parameter where the strides are written.
	 */
	REXLIB_API
	void get_strides(std::vector<std::ptrdiff_t> &strides) const;

	/**
	 * @brief Get the offset of the data.
	 * 
	 * @return std::size_t The offset in elements.
	 */
	REXLIB_API
	std::ptrdiff_t get_offset() const noexcept;

	/**
	 * @brief Computes the minimum storage size (in elements) for this layout.
	 * 
	 * @return std::size_t Minimum storage size in elements.
	 */
	REXLIB_API
	std::size_t compute_storage_requirement() const noexcept;

	/**
	 * @brief Computes the number of elements referenced by the layout.
	 * 
	 * @return std::size_t Number of elements.
	 */
	REXLIB_API
	std::size_t compute_element_count() const noexcept;

	/**
	 * @brief Check if this layout has a specific shape.
	 * 
	 * @param extents The extents to be compared.
	 * @return true If the provided extents match layout's extents.
	 * @return false If the provided extents do not match layout's extents.
	 */
	REXLIB_API
	bool extents_equal(span<const std::size_t> extents) const noexcept;

	/**
	 * @brief Apply a set of dynamic subscripts to this layout.
	 * 
	 * @param subscripts The subscripts.
	 * @return strided_layout The resulting layout.
	 * @throws std::invalid_argument If not all subscripts are processed.
	 * Or subscript is out of bounds
	 */
	REXLIB_NODISCARD REXLIB_API
	strided_layout 
	apply_subscripts(span<const dynamic_subscript> subscripts) const;

	/**
	 * @brief Reverse the order of the axes.
	 * 
	 * @return strided_layout The resulting layout.
	 */
	REXLIB_NODISCARD REXLIB_API
	strided_layout transpose() const;

	/**
	 * @brief Permute the order of the axes.
	 * 
	 * @param order Order acquired by the new layout. Must have the same 
	 * size as the amount of dimensions and it must feature strictly one
	 * instance of each number in [0, rank).
	 * @return strided_layout Permuted layout.
	 * @throws std::invalid_argument If the permutation order is invalid.
	 */
	REXLIB_NODISCARD REXLIB_API
	strided_layout permute(span<const std::size_t> order) const;

	/**
	 * @brief Swap two axes in the layout.
	 * 
	 * @param axis1 Index of the first axis. Must be in [-rank, rank).
	 * @param axis2 Index of the second axis. Must be in [-rank, rank).
	 * @return strided_layout Permuted layout.
	 * @throws std::out_of_range If either axis1 or axis2 exceeds bounds.
	 */
	REXLIB_NODISCARD REXLIB_API
	strided_layout matrix_transpose(
		std::ptrdiff_t axis1 = -1, 
		std::ptrdiff_t axis2 = -2
	) const;

	/**
	 * @brief Obtains a layout view to the matrix diagonal defined by two axes.
	 * 
	 * @param axis1 First axis of the diagonal. Must be in [-rank, rank).
	 * @param axis1 Second axis of the diagonal. Must be in [-rank, rank).
	 * Cannot be equal to axis1.
	 * @return strided_layout The resulting layout of the matrix_diagonal.
	 * @throws std::out_of_range If either axis1 or axis2 exceeds bounds.
	 * @throws std::invalid_argument If axis1 and axis2 are equal.
	 */
	REXLIB_NODISCARD REXLIB_API
	strided_layout matrix_diagonal(
		std::ptrdiff_t axis1 = -1, 
		std::ptrdiff_t axis2 = -2
	) const;

	/**
	 * @brief Remove insignificant axes of the layout.
	 * 
	 * @return strided_layout The resulting layout.
	 */
	REXLIB_NODISCARD REXLIB_API
	strided_layout squeeze() const;

	/**
	 * @brief Perform a broadcast of the layout to match the provided extents.
	 * 
	 * This function modifies the layout to match the provided extents by 
	 * adding phantom axes or adjusting existing axes.
	 * 
	 * @param extents Extents to broadcast to.
	 * untouched. Must be greater or equal to rank. Defaults to zero.
	 * @return strided_layout The resulting broadcasted layout.
	 * @throws std::invalid_argument If the layout has more axes than extents.
	 * @throws std::invalid_argument If the axes cannot be broadcasted to the 
	 * provided extents.
	 */
	REXLIB_NODISCARD REXLIB_API
	strided_layout broadcast_to(span<const std::size_t> extents) const;

	/**
	 * @brief Get the implementation of this layout.
	 * 
	 * @return const strided_layout_implementation& The implementation 
	 * reference.
	 *
	 * @note The implementation class is not part of the public API. This method
	 * is provided for internal use and testing purposes.
	 */
	const strided_layout_implementation& get_implementation() const noexcept;

	/**
	 * @brief Create a contiguous layout from the provided extents.
	 * 
	 * @param extents Extents of the layout.
	 * @return strided_layout The resulting layout.
	 */
	REXLIB_API
	static
	strided_layout make_contiguous_layout(span<const std::size_t> extents);

	REXLIB_API
	static
	strided_layout make_custom_layout(
		span<const std::size_t> extents,
		span<const std::ptrdiff_t> strides,
		std::ptrdiff_t offset = 0
	);

	// Defined out of line: it compares the implementations, and the
	// implementation type is incomplete here.
	friend REXLIB_API
	bool operator==(const strided_layout &lhs, const strided_layout &rhs) noexcept;

	friend bool
	operator!=(const strided_layout &lhs, const strided_layout &rhs) noexcept
	{
		return !(lhs == rhs);
	}

private:
	// Copy-on-write implementation
	std::shared_ptr<const strided_layout_implementation> m_implementation; 

	explicit 
	strided_layout(
		std::shared_ptr<const strided_layout_implementation> impl
	) noexcept;
	explicit 
	strided_layout(strided_layout_implementation &&impl);
};

} // namespace rexlib

namespace std
{

template<>
struct hash<rexlib::strided_layout>
{
	std::size_t operator()(
		const rexlib::strided_layout &layout
	) const noexcept
	{
		return layout.hash();
	}
};

} // namespace std
