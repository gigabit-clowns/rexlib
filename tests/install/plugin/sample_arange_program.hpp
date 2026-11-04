// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "sample_program.hpp"

#include <cstddef>
#include <cstdint>

namespace sample
{

/**
 * @brief Writes the progression `start + i * step` into a one dimensional
 * operand of int32.
 */
class sample_arange_program final
	: public sample_program
{
public:
	/**
	 * @brief Construct the program.
	 *
	 * @param start Value of the first element.
	 * @param step Difference between one element and the next.
	 * @param count Number of elements to write.
	 * @param offset Position of the first element in the output buffer, in
	 * elements.
	 * @param stride Distance between one element and the next in the output
	 * buffer, in elements.
	 */
	sample_arange_program(
		std::int32_t start,
		std::int32_t step,
		std::size_t count,
		std::ptrdiff_t offset,
		std::ptrdiff_t stride
	) noexcept;

	void execute(
		rexlib::span<const std::shared_ptr<rexlib::buffer>> outputs,
		rexlib::span<const std::shared_ptr<const rexlib::buffer>> inputs,
		rexlib::span<const std::shared_ptr<rexlib::buffer>> scratch
	) const override;

private:
	std::int32_t m_start;
	std::int32_t m_step;
	std::size_t m_count;
	std::ptrdiff_t m_offset;
	std::ptrdiff_t m_stride;
};

} // namespace sample
