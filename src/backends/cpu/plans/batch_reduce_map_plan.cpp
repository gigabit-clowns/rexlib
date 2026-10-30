// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/plans/batch_reduce_map_plan.hpp>

#include <backends/cpu/plans/index_operands.hpp>

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

named_core_frame make_frame(
	span<const operand_descriptor> output_descriptors,
	span<const std::size_t> named_axes
)
{
	if (output_descriptors.empty())
	{
		throw std::invalid_argument(
			"batch_reduce_map_plan: At least one output operand is needed."
		);
	}

	std::vector<std::size_t> extents;
	output_descriptors[0].get_layout().get_extents(extents);
	return named_core_frame(std::move(extents), named_axes);
}

void check_cores(
	span<const operand_descriptor> input_descriptors,
	span<const core_dimensions_descriptor> input_cores
)
{
	if (input_cores.size() != input_descriptors.size())
	{
		throw std::invalid_argument(
			"batch_reduce_map_plan: Every input operand needs a core, and "
			"only they do."
		);
	}

	const auto carrying = std::find_if(
		input_cores.begin(),
		input_cores.end(),
		[](const core_dimensions_descriptor &core) { return !core.is_empty(); }
	);
	if (carrying == input_cores.end())
	{
		throw std::invalid_argument(
			"batch_reduce_map_plan: No input operand carries the core axes."
		);
	}
}

std::size_t element_count(const std::vector<std::size_t> &extents) noexcept
{
	std::size_t result = 1;
	for (const auto extent : extents)
	{
		result *= extent;
	}
	return result;
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
 * @brief Add the core axes of an aligned operand to a core layout, without
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
 * @brief The axes of the result along which every input without core axes
 * holds a single value.
 */
std::vector<std::size_t> find_shared_axes(
	const named_core_frame &frame,
	span<const operand_descriptor> input_descriptors,
	span<const core_dimensions_descriptor> input_cores
)
{
	const auto rank = frame.get_frame_extents().size();
	std::vector<bool> shared(rank, true);

	std::vector<std::size_t> extents;
	for (std::size_t i = 0; i < input_descriptors.size(); ++i)
	{
		if (!input_cores[i].is_empty())
		{
			continue;
		}

		frame.embed(input_descriptors[i].get_layout(), false)
			.get_extents(extents);
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

void add_counter(
	const named_core_frame& /*frame*/,
	span<const operand_descriptor> /*input_descriptors*/,
	span<const core_dimensions_descriptor> /*input_cores*/,
	joint_layout_builder& /*batch_builder*/,
	joint_layout_builder& /*map_builder*/,
	no_index_tag
) noexcept
{
	// No counter to be added.
}

void add_counter(
	const named_core_frame &frame,
	span<const operand_descriptor> input_descriptors,
	span<const core_dimensions_descriptor> input_cores,
	joint_layout_builder &batch_builder,
	joint_layout_builder &map_builder,
	linear_index_tag
)
{
	const auto shared_axes =
		find_shared_axes(frame, input_descriptors, input_cores);
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

} // anonymous namespace

template <typename ReductionIndexing, typename MapIndexing>
batch_reduce_map_plan::batch_reduce_map_plan(
	span<const operand_descriptor> output_descriptors,
	span<const operand_descriptor> input_descriptors,
	span<const core_dimensions_descriptor> input_cores,
	span<const std::size_t> named_axes,
	std::size_t statistic_count,
	ReductionIndexing reduction_indexing,
	MapIndexing map_indexing
)
	: m_frame(make_frame(output_descriptors, named_axes))
	, m_batch_count(0)
	, m_core_count(0)
{
	check_cores(input_descriptors, input_cores);

	const auto batch_extents = m_frame.get_batch_extents();
	const auto core_extents = m_frame.get_core_extents();

	joint_layout_builder batch_builder;
	joint_layout_builder scalar_builder;
	joint_layout_builder reduced_builder;
	joint_layout_builder map_builder;
	batch_builder.set_extents(make_span(batch_extents));
	scalar_builder.set_extents(span<const std::size_t>());
	reduced_builder.set_extents(make_span(core_extents));
	map_builder.set_extents(make_span(core_extents));

	std::vector<strided_layout> aligned_inputs;
	aligned_inputs.reserve(input_descriptors.size());
	for (std::size_t i = 0; i < input_descriptors.size(); ++i)
	{
		const auto carries_core = !input_cores[i].is_empty();
		aligned_inputs.push_back(
			m_frame.align(input_descriptors[i].get_layout(), carries_core)
		);

		add_batch_operand(m_frame, aligned_inputs.back(), batch_builder);
		if (carries_core)
		{
			add_core_operand(m_frame, aligned_inputs.back(), reduced_builder);
			scalar_builder.add_operand(
				span<const std::size_t>(),
				span<const std::ptrdiff_t>(),
				0
			);
		}
	}
	add_index_operands(
		reduced_builder,
		make_span(core_extents),
		reduction_indexing
	);
	for (std::size_t s = 0; s < statistic_count; ++s)
	{
		scalar_builder.add_operand(
			span<const std::size_t>(),
			span<const std::ptrdiff_t>(),
			0
		);
	}

	for (const auto &descriptor : output_descriptors)
	{
		const auto aligned = m_frame.align(descriptor.get_layout(), true);
		add_batch_operand(m_frame, aligned, batch_builder);
		add_core_operand(m_frame, aligned, map_builder);
	}
	for (std::size_t i = 0; i < input_descriptors.size(); ++i)
	{
		if (!input_cores[i].is_empty())
		{
			add_core_operand(m_frame, aligned_inputs[i], map_builder);
		}
	}
	add_counter(
		m_frame,
		input_descriptors,
		input_cores,
		batch_builder,
		map_builder,
		map_indexing
	);

	m_batch_layout = batch_builder.build();
	m_scalar_layout = scalar_builder.build();
	m_reduced_layout = reduced_builder.build();
	m_map_layout = map_builder.build();
	m_batch_count = element_count(batch_extents);
	m_core_count = element_count(core_extents);
}

const named_core_frame& batch_reduce_map_plan::get_frame() const noexcept
{
	return m_frame;
}

const joint_layout& batch_reduce_map_plan::get_batch_layout() const noexcept
{
	return m_batch_layout;
}

const joint_layout& batch_reduce_map_plan::get_scalar_layout() const noexcept
{
	return m_scalar_layout;
}

const joint_layout&
batch_reduce_map_plan::get_reduced_layout() const noexcept
{
	return m_reduced_layout;
}

const joint_layout& batch_reduce_map_plan::get_map_layout() const noexcept
{
	return m_map_layout;
}

std::size_t batch_reduce_map_plan::get_batch_count() const noexcept
{
	return m_batch_count;
}

std::size_t batch_reduce_map_plan::get_core_count() const noexcept
{
	return m_core_count;
}

#define REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN(reduction, map) \
	template batch_reduce_map_plan::batch_reduce_map_plan( \
		span<const operand_descriptor>, \
		span<const operand_descriptor>, \
		span<const core_dimensions_descriptor>, \
		span<const std::size_t>, \
		std::size_t, \
		reduction, \
		map \
	)

REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN(no_index_tag, no_index_tag);
REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN(no_index_tag, linear_index_tag);
REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN(linear_index_tag, no_index_tag);
REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN(linear_index_tag, linear_index_tag);
REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN(
	multidimensional_index_tag,
	no_index_tag
);
REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN(
	multidimensional_index_tag,
	linear_index_tag
);

#undef REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN

} // namespace cpu
} // namespace rexlib
