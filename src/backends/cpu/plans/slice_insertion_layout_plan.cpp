// SPDX-License-Identifier: GPL-3.0-only

#include "slice_insertion_layout_plan.hpp"

#include <core/layout/axis_values.hpp>

#include <rexlib/core/layout/broadcast.hpp>
#include <rexlib/core/layout/joint_layout_builder.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/platform/assert.hpp>

#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace cpu
{

namespace
{

/**
 * @brief The batch prefix of an operand, as a layout of its own.
 */
class batch_prefix
{
public:
	batch_prefix(
		const operand_descriptor &descriptor,
		std::size_t core_rank,
		std::vector<operand_core> &cores
	)
	{
		std::vector<std::size_t> extents;
		std::vector<std::ptrdiff_t> strides;
		cores.push_back(
			split_trailing_core(
				descriptor.get_layout(),
				core_rank,
				extents,
				strides
			)
		);
		m_layout = strided_layout::make_custom_layout(
			make_span(extents),
			make_span(strides),
			descriptor.get_layout().get_offset()
		);
	}

	const strided_layout& get_layout() const noexcept
	{
		return m_layout;
	}

private:
	strided_layout m_layout;
};

void add_operand(
	joint_layout_builder &builder,
	const strided_layout &layout,
	const std::vector<std::size_t> &axes,
	const std::vector<std::size_t> &axis_extents,
	std::ptrdiff_t offset
)
{
	std::vector<std::ptrdiff_t> strides;
	layout.get_strides(strides);
	const auto selected = select_axes(make_span(strides), make_span(axes));
	builder.add_operand(
		make_span(axis_extents),
		make_span(selected),
		offset
	);
}

} // anonymous namespace

slice_insertion_layout_plan::slice_insertion_layout_plan(
	span<const operand_descriptor> output_descriptors,
	span<const operand_descriptor> input_descriptors,
	span<const std::size_t> input_core_ranks
)
{
	REXLIB_ASSERT(output_descriptors.size() == 2);
	REXLIB_ASSERT(input_descriptors.size() == input_core_ranks.size());
	REXLIB_ASSERT(input_descriptors.size() >= 3);

	std::vector<batch_prefix> input_prefixes;
	input_prefixes.reserve(input_descriptors.size());
	for (std::size_t i = 0; i < input_descriptors.size(); ++i)
	{
		input_prefixes.emplace_back(
			input_descriptors[i],
			input_core_ranks[i],
			m_input_cores
		);
	}

	std::vector<batch_prefix> output_prefixes;
	output_prefixes.reserve(output_descriptors.size());
	for (const auto &descriptor : output_descriptors)
	{
		output_prefixes.emplace_back(descriptor, 3, m_output_cores);
	}

	m_volume_extent = m_input_cores[0].get_extent(0);
	m_slice_extent = m_input_cores[2].get_extent(0);

	std::vector<std::size_t> frame;
	std::vector<std::size_t> extents;
	for (const auto &prefix : input_prefixes)
	{
		prefix.get_layout().get_extents(extents);
		broadcast_extents_accumulate(frame, make_span(extents));
	}

	// An axis the accumulators lack, or hold with an extent of one, is
	// reduced into them.
	const auto accumulator =
		input_prefixes[0].get_layout().broadcast_to(make_span(frame));
	std::vector<std::ptrdiff_t> accumulator_strides;
	accumulator.get_strides(accumulator_strides);
	std::vector<std::size_t> kept_axes;
	std::vector<std::size_t> reduced_axes;
	for (std::size_t axis = 0; axis < frame.size(); ++axis)
	{
		if (accumulator_strides[axis] == 0 && frame[axis] > 1)
		{
			reduced_axes.push_back(axis);
		}
		else
		{
			kept_axes.push_back(axis);
		}
	}

	const auto kept_extents =
		select_axes(make_span(frame), make_span(kept_axes));
	const auto reduced_extents =
		select_axes(make_span(frame), make_span(reduced_axes));
	joint_layout_builder kept_builder;
	joint_layout_builder reduced_builder;
	kept_builder.set_extents(make_span(kept_extents));
	reduced_builder.set_extents(make_span(reduced_extents));

	for (const auto &prefix : input_prefixes)
	{
		const auto layout = prefix.get_layout().broadcast_to(make_span(frame));
		add_operand(
			kept_builder,
			layout,
			kept_axes,
			kept_extents,
			layout.get_offset()
		);
		add_operand(reduced_builder, layout, reduced_axes, reduced_extents, 0);
	}
	for (const auto &prefix : output_prefixes)
	{
		const auto layout = prefix.get_layout().broadcast_to(make_span(frame));
		add_operand(
			kept_builder,
			layout,
			kept_axes,
			kept_extents,
			layout.get_offset()
		);
	}

	m_kept_layout = kept_builder.build();
	m_reduced_layout = reduced_builder.build();
	m_kept_count = m_kept_layout.compute_element_count();
	m_reduced_count = m_reduced_layout.compute_element_count();

	for (std::size_t o = 0; o < output_descriptors.size(); ++o)
	{
		m_output_matches_accumulator.push_back(
			output_descriptors[o].get_layout() ==
			input_descriptors[o].get_layout()
		);
	}
}

std::size_t slice_insertion_layout_plan::get_volume_extent() const noexcept
{
	return m_volume_extent;
}

std::size_t slice_insertion_layout_plan::get_slice_extent() const noexcept
{
	return m_slice_extent;
}

const operand_core& slice_insertion_layout_plan::get_output_core(
	std::size_t output
) const noexcept
{
	return m_output_cores[output];
}

const operand_core& slice_insertion_layout_plan::get_input_core(
	std::size_t input
) const noexcept
{
	return m_input_cores[input];
}

const joint_layout&
slice_insertion_layout_plan::get_kept_layout() const noexcept
{
	return m_kept_layout;
}

const joint_layout&
slice_insertion_layout_plan::get_reduced_layout() const noexcept
{
	return m_reduced_layout;
}

std::size_t slice_insertion_layout_plan::get_kept_count() const noexcept
{
	return m_kept_count;
}

std::size_t slice_insertion_layout_plan::get_reduced_count() const noexcept
{
	return m_reduced_count;
}

bool slice_insertion_layout_plan::output_matches_accumulator(
	std::size_t output
) const noexcept
{
	return m_output_matches_accumulator[output];
}

} // namespace cpu
} // namespace rexlib
