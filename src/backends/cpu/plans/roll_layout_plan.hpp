// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief A cyclic shift along one axis.
 *
 * The shift is how many elements the output's first index along the axis
 * is offset from the input's, e.g. `output[0] == input[shift]` along that
 * axis alone. It always lies in `(0, extent)`: a shift of zero is the
 * identity and has no reason to appear here.
 */
class roll_axis_shift
{
public:
	roll_axis_shift(std::size_t axis, std::size_t shift) noexcept;

	std::size_t get_axis() const noexcept;
	std::size_t get_shift() const noexcept;

private:
	std::size_t m_axis;
	std::size_t m_shift;
};

/**
 * @brief Build the blocks a cyclic axis shift decomposes into.
 *
 * A shift along one axis reads a contiguous range from the tail of the
 * input into the head of the output and another from the head of the input
 * into the tail of the output; neither range wraps on its own. Shifting
 * several axes at once combines these choices independently, one of two
 * per shifted axis, so the whole operation decomposes into
 * `2^shifts.size()` blocks, each a plain, non-wrapping affine copy that
 * @ref run_elementwise_loop can drive directly.
 *
 * @param extents The extents shared by the output and the input: a cyclic
 * shift does not change shape.
 * @param output_strides,output_offset The output operand's layout.
 * @param input_strides,input_offset The input operand's layout.
 * @param shifts The axes being shifted, with axes of zero shift already
 * filtered out.
 * @return One joint_layout per block, output operand first and input
 * operand second, matching the pointer order @ref run_elementwise_loop
 * expects.
 */
std::vector<joint_layout> build_roll_blocks(
	span<const std::size_t> extents,
	span<const std::ptrdiff_t> output_strides,
	std::ptrdiff_t output_offset,
	span<const std::ptrdiff_t> input_strides,
	std::ptrdiff_t input_offset,
	span<const roll_axis_shift> shifts
);

} // namespace cpu
} // namespace rexlib
