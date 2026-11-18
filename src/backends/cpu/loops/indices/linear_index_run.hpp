// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The linear index of an element, and how it changes along the
 * innermost axis of a traversal.
 *
 * @tparam Stride Change of the linear index between consecutive elements of
 * the innermost axis. Either a stride tag or a runtime `std::ptrdiff_t`.
 */
template <typename Stride>
class linear_index_run
{
public:
	/**
	 * @brief Construct a run.
	 *
	 * @param index Linear index of the element the run is at.
	 * @param stride Change of the linear index between consecutive elements.
	 */
	REXLIB_CONSTEXPR linear_index_run(
		std::ptrdiff_t index,
		Stride stride
	) noexcept;

	/**
	 * @brief Get the linear index of the element the run is at.
	 *
	 * @return std::size_t The linear index.
	 */
	REXLIB_CONSTEXPR std::size_t get_index() const noexcept;

	/**
	 * @brief Get this run placed at the element a cursor points to.
	 *
	 * @param index_offsets Offsets of the cursor, starting at the linear index
	 * operand.
	 * @return linear_index_run The run at that element.
	 */
	REXLIB_CONSTEXPR linear_index_run rebased(
		const std::ptrdiff_t *index_offsets
	) const noexcept;

	/**
	 * @brief Get this run a number of elements further along the innermost
	 * axis.
	 *
	 * @param count Number of elements to advance by.
	 * @return linear_index_run The run at that element.
	 */
	REXLIB_CONSTEXPR
	linear_index_run advanced(std::size_t count) const noexcept;

private:
	std::ptrdiff_t m_index;
	Stride m_stride;
};

} // namespace cpu
} // namespace rexlib

#include "linear_index_run.inl"
