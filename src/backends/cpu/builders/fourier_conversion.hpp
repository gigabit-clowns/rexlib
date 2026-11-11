// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/plans/fourier_layout_plan.hpp>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Write an operand into another one, converting element by element.
 *
 * A real element becomes a complex one with no imaginary part, and a complex
 * element is copied across. Both operands must share their extents, so this
 * only serves a transform between two complex arrays.
 *
 * @tparam T The destination element type.
 * @tparam Q The source element type.
 * @param plan The planned transform.
 * @param output The destination, displaced by its offset.
 * @param input The source, displaced by its offset.
 * @param schedule The threads to spread the conversion over.
 */
template <typename T, typename Q>
void run_fourier_conversion(
	const fourier_layout_plan &plan,
	T *output,
	const Q *input,
	const loop_schedule &schedule
);

} // namespace cpu
} // namespace rexlib

#include "fourier_conversion.inl"
