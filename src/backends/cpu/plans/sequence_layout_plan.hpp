// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rex
{

class strided_layout;

namespace cpu
{

/**
 * @brief The traversal a sequence program performs, settled at build time.
 *
 * A sequence is written along a single axis, so the whole of its iteration
 * is three numbers. Reading them off the layout costs two vector
 * allocations, which is why it happens once here, when the program is built,
 * rather than every time it runs.
 */
class sequence_layout_plan
{
public:
	/**
	 * @brief Plan the traversal of a one dimensional operand.
	 *
	 * @param layout Layout of the operand. It must have rank one.
	 *
	 * @throws std::invalid_argument If the layout is not one dimensional.
	 */
	explicit sequence_layout_plan(const strided_layout &layout);

	/**
	 * @brief Get the offset of the first element.
	 *
	 * @return std::ptrdiff_t The offset, in elements.
	 */
	std::ptrdiff_t get_offset() const noexcept;

	/**
	 * @brief Get the number of elements.
	 *
	 * @return std::size_t The element count.
	 */
	std::size_t get_count() const noexcept;

	/**
	 * @brief Get the step between consecutive elements.
	 *
	 * @return std::ptrdiff_t The stride, in elements.
	 */
	std::ptrdiff_t get_stride() const noexcept;

private:
	std::ptrdiff_t m_offset;
	std::size_t m_count;
	std::ptrdiff_t m_stride;
};

} // namespace cpu
} // namespace rex
