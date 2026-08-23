// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../span.hpp"
#include "../platform/dynamic_shared_object.h"

#include <vector>

namespace rex 
{

/**
 * @brief Traversal state (indices and per-operand offsets) of a joint_layout.
 */
class joint_cursor
{
public:
	/**
	 * @brief Default constructor.
	 */
	REXLIB_API
	joint_cursor() noexcept;

	/**
	 * @brief Construct a cursor at its initial state.
	 * 
	 * @param ndim Number of dimensions. Initial indices will correspond to 
	 * this amount of zeros.
	 * @param offsets The base offsets for each operand.
	 */
	REXLIB_API
	joint_cursor(
		std::size_t ndim,
		std::vector<std::ptrdiff_t> offsets
	);

	REXLIB_API
	joint_cursor(const joint_cursor &other);
	REXLIB_API
	joint_cursor(joint_cursor &&other) noexcept;
	REXLIB_API
	~joint_cursor();

	REXLIB_API
	joint_cursor& operator=(const joint_cursor &other);
	REXLIB_API
	joint_cursor& operator=(joint_cursor &&other) noexcept;

	/**
	 * @brief Get the current multidimensional indices.
	 * 
	 * Unless assigned to another iterator, the pointed span is guaranteed to 
	 * remain constant.
	 * 
	 * @return span<const std::size_t> The indices.
	 */
	REXLIB_API
	span<const std::size_t> get_indices() const noexcept;

	/**
	 * @brief Get the current multidimensional indices.
	 * 
	 * Unless assigned to another iterator, the pointed span is guaranteed to 
	 * remain constant.
	 * 
	 * @return span<std::size_t> The indices.
	 */
	REXLIB_API
	span<std::size_t> get_indices() noexcept;

	/**
	 * @brief Get the current offsets for each operand.
	 * 
	 * Unless assigned to another iterator, the pointed span is guaranteed to 
	 * remain constant.
	 * 
	 * @return span<const std::ptrdiff_t> The offsets.
	 */
	REXLIB_API
	span<const std::ptrdiff_t> get_offsets() const noexcept;

	/**
	 * @brief Get the current offsets for each operand.
	 * 
	 * Unless assigned to another iterator, the pointed span is guaranteed to 
	 * remain constant.
	 * 
	 * @return span<std::ptrdiff_t> The offsets.
	 */
	REXLIB_API
	span<std::ptrdiff_t> get_offsets() noexcept;

private:
	std::vector<std::size_t> m_indices;
	std::vector<std::ptrdiff_t> m_offsets;
};

} // namespace rex
