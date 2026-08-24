// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/plans/roll_layout_plan.hpp>

#include <rexlib/core/layout/joint_layout_builder.hpp>

namespace rexlib
{
namespace cpu
{

roll_axis_shift::roll_axis_shift(std::size_t axis, std::size_t shift) noexcept
	: m_axis(axis)
	, m_shift(shift)
{
}

std::size_t roll_axis_shift::get_axis() const noexcept
{
	return m_axis;
}

std::size_t roll_axis_shift::get_shift() const noexcept
{
	return m_shift;
}

std::vector<joint_layout> build_roll_blocks(
	span<const std::size_t> extents,
	span<const std::ptrdiff_t> output_strides,
	std::ptrdiff_t output_offset,
	span<const std::ptrdiff_t> input_strides,
	std::ptrdiff_t input_offset,
	span<const roll_axis_shift> shifts
)
{
	const auto num_shifted = shifts.size();
	const std::size_t num_blocks = std::size_t(1) << num_shifted;

	std::vector<std::size_t> block_extents(extents.begin(), extents.end());
	std::vector<joint_layout> blocks;
	blocks.reserve(num_blocks);

	for (std::size_t combo = 0; combo < num_blocks; ++combo)
	{
		std::ptrdiff_t out_extra_offset = 0;
		std::ptrdiff_t in_extra_offset = 0;

		for (std::size_t i = 0; i < num_shifted; ++i)
		{
			const auto &s = shifts[i];
			const auto axis = s.get_axis();
			const auto shift = s.get_shift();
			const auto extent = extents[axis];

			// Bit i chosen: the output's head reads from the input's tail
			// (the part that wraps around). Otherwise, the output's tail
			// reads from the input's head (the part that does not move).
			if ((combo >> i) & std::size_t(1))
			{
				block_extents[axis] = shift;
				in_extra_offset +=
					static_cast<std::ptrdiff_t>(extent - shift) *
					input_strides[axis];
			}
			else
			{
				block_extents[axis] = extent - shift;
				out_extra_offset +=
					static_cast<std::ptrdiff_t>(shift) *
					output_strides[axis];
			}
		}

		const auto block_extents_span = make_span(block_extents);

		joint_layout_builder builder;
		builder.add_operand(
			block_extents_span,
			output_strides,
			output_offset + out_extra_offset
		);
		builder.add_operand(
			block_extents_span,
			input_strides,
			input_offset + in_extra_offset
		);
		blocks.push_back(builder.build());
	}

	return blocks;
}

} // namespace cpu
} // namespace rexlib
