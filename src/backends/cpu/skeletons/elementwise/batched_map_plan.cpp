// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/skeletons/elementwise/batched_map_plan.hpp>

#include <backends/cpu/loops/layouts/index_operands.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/joint_layout_builder.hpp>
#include <rexlib/core/layout/strided_layout.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rexlib
{
namespace cpu
{

namespace
{

void check_operands(
	span<const operand_descriptor> output_descriptors,
	span<const operand_descriptor> input_descriptors,
	span<const core_dimensions_descriptor> input_cores
)
{
	if (output_descriptors.empty())
	{
		throw std::invalid_argument(
			"batched_map_plan: At least one output operand is needed."
		);
	}
	if (input_cores.size() != input_descriptors.size())
	{
		throw std::invalid_argument(
			"batched_map_plan: Every input operand needs a core, and only "
			"they do."
		);
	}
}

bool is_mapped(span<const std::size_t> mapped_inputs, std::size_t input)
{
	return std::find(mapped_inputs.begin(), mapped_inputs.end(), input) !=
		mapped_inputs.end();
}

/**
 * @brief Split the core of its own off an input, leaving its batch axes.
 */
strided_layout split_own_core(
	const strided_layout &layout,
	const core_dimensions_descriptor &core,
	operand_core &own_core
)
{
	if (core.has_run())
	{
		throw std::invalid_argument(
			"batched_map_plan: An input read once per batch element cannot "
			"lead its core with a run of axes."
		);
	}

	std::vector<std::size_t> batch_extents;
	std::vector<std::ptrdiff_t> batch_strides;
	own_core = split_trailing_core(
		layout,
		core.get_slots().size(),
		batch_extents,
		batch_strides
	);
	return strided_layout::make_custom_layout(
		make_span(batch_extents),
		make_span(batch_strides),
		layout.get_offset()
	);
}

/**
 * @brief Add the batch axes of an aligned operand to the batch layout, with
 * its offset.
 */
void add_batch_operand(
	const named_core_frame &frame,
	const strided_layout &aligned,
	joint_layout_builder &builder
)
{
	std::vector<std::ptrdiff_t> strides;
	aligned.get_strides(strides);

	const auto extents = frame.get_batch_extents();
	const auto batch_strides = frame.select_batch(make_span(strides));
	builder.add_operand(
		make_span(extents),
		make_span(batch_strides),
		aligned.get_offset()
	);
}

/**
 * @brief Add the core axes of an aligned operand to the map layout, without
 * its offset.
 */
void add_core_operand(
	const named_core_frame &frame,
	const strided_layout &aligned,
	joint_layout_builder &builder
)
{
	std::vector<std::ptrdiff_t> strides;
	aligned.get_strides(strides);

	const auto extents = frame.get_core_extents();
	const auto core_strides = frame.select_core(make_span(strides));
	builder.add_operand(make_span(extents), make_span(core_strides), 0);
}

/**
 * @brief The axes of the result along which every input read once per
 * batch element holds a single value.
 */
std::vector<std::size_t> find_shared_axes(
	const named_core_frame &frame,
	span<const strided_layout> own_core_batches
)
{
	const auto rank = frame.get_frame_extents().size();
	std::vector<bool> shared(rank, true);

	std::vector<std::size_t> extents;
	for (const auto &batch : own_core_batches)
	{
		frame.embed(batch, false).get_extents(extents);
		for (std::size_t axis = 0; axis < rank; ++axis)
		{
			shared[axis] = shared[axis] && extents[axis] == 1;
		}
	}

	std::vector<std::size_t> result;
	for (std::size_t axis = 0; axis < rank; ++axis)
	{
		if (shared[axis])
		{
			result.push_back(axis);
		}
	}
	return result;
}

void add_map_index(
	const named_core_frame& /*frame*/,
	span<const strided_layout> /*own_core_batches*/,
	joint_layout_builder& /*batch_builder*/,
	joint_layout_builder& /*map_builder*/,
	no_index_tag
) noexcept
{
	// No index to be added.
}

void add_map_index(
	const named_core_frame &frame,
	span<const strided_layout> own_core_batches,
	joint_layout_builder &batch_builder,
	joint_layout_builder &map_builder,
	linear_index_tag
)
{
	const auto shared_axes = find_shared_axes(frame, own_core_batches);
	const auto strides = make_row_major_strides(
		frame.get_frame_extents(),
		make_span(shared_axes)
	);

	const auto counter = strided_layout::make_custom_layout(
		frame.get_frame_extents(),
		make_span(strides)
	);
	add_batch_operand(frame, counter, batch_builder);
	add_core_operand(frame, counter, map_builder);
}

void add_map_index(
	const named_core_frame &frame,
	span<const strided_layout> /*own_core_batches*/,
	joint_layout_builder& /*batch_builder*/,
	joint_layout_builder &map_builder,
	multidimensional_index_tag indexing
)
{
	const auto core_extents = frame.get_core_extents();
	add_index_operands(map_builder, make_span(core_extents), indexing);
}

} // anonymous namespace

template <typename MapIndexing>
batched_map_plan::batched_map_plan(
	named_core_frame frame,
	span<const operand_descriptor> output_descriptors,
	span<const operand_descriptor> input_descriptors,
	span<const core_dimensions_descriptor> input_cores,
	span<const std::size_t> mapped_inputs,
	MapIndexing map_indexing
)
	: m_frame(std::move(frame))
	, m_input_cores(input_descriptors.size())
	, m_batch_count(0)
	, m_core_count(0)
{
	check_operands(output_descriptors, input_descriptors, input_cores);

	const auto batch_extents = m_frame.get_batch_extents();
	const auto core_extents = m_frame.get_core_extents();

	joint_layout_builder batch_builder;
	joint_layout_builder map_builder;
	batch_builder.set_extents(make_span(batch_extents));
	map_builder.set_extents(make_span(core_extents));

	std::vector<strided_layout> aligned_inputs;
	std::vector<strided_layout> own_core_batches;
	aligned_inputs.reserve(input_descriptors.size());
	for (std::size_t i = 0; i < input_descriptors.size(); ++i)
	{
		const auto &layout = input_descriptors[i].get_layout();
		if (is_mapped(mapped_inputs, i))
		{
			aligned_inputs.push_back(
				m_frame.align(layout, !input_cores[i].is_empty())
			);
		}
		else
		{
			own_core_batches.push_back(
				split_own_core(layout, input_cores[i], m_input_cores[i])
			);
			aligned_inputs.push_back(
				m_frame.align(own_core_batches.back(), false)
			);
		}
		add_batch_operand(m_frame, aligned_inputs.back(), batch_builder);
	}

	for (const auto &descriptor : output_descriptors)
	{
		const auto aligned = m_frame.align(descriptor.get_layout(), true);
		add_batch_operand(m_frame, aligned, batch_builder);
		add_core_operand(m_frame, aligned, map_builder);
	}
	for (const auto input : mapped_inputs)
	{
		add_core_operand(m_frame, aligned_inputs[input], map_builder);
	}
	add_map_index(
		m_frame,
		make_span(own_core_batches),
		batch_builder,
		map_builder,
		map_indexing
	);

	m_batch_layout = batch_builder.build();
	m_map_layout = map_builder.build();
	m_batch_count = m_batch_layout.compute_element_count();
	m_core_count = m_map_layout.compute_element_count();
}

const named_core_frame& batched_map_plan::get_frame() const noexcept
{
	return m_frame;
}

const operand_core&
batched_map_plan::get_input_core(std::size_t input) const noexcept
{
	return m_input_cores[input];
}

const joint_layout& batched_map_plan::get_batch_layout() const noexcept
{
	return m_batch_layout;
}

const joint_layout& batched_map_plan::get_map_layout() const noexcept
{
	return m_map_layout;
}

std::size_t batched_map_plan::get_batch_count() const noexcept
{
	return m_batch_count;
}

std::size_t batched_map_plan::get_core_count() const noexcept
{
	return m_core_count;
}

#define REXLIB_INSTANTIATE_BATCHED_MAP_PLAN(map) \
	template batched_map_plan::batched_map_plan( \
		named_core_frame, \
		span<const operand_descriptor>, \
		span<const operand_descriptor>, \
		span<const core_dimensions_descriptor>, \
		span<const std::size_t>, \
		map \
	)

REXLIB_INSTANTIATE_BATCHED_MAP_PLAN(no_index_tag);
REXLIB_INSTANTIATE_BATCHED_MAP_PLAN(linear_index_tag);
REXLIB_INSTANTIATE_BATCHED_MAP_PLAN(multidimensional_index_tag);

#undef REXLIB_INSTANTIATE_BATCHED_MAP_PLAN

} // namespace cpu
} // namespace rexlib
