// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/skeletons/batch_reduce_map/batch_reduce_map_plan.hpp>

#include <backends/cpu/loops/layouts/index_operands.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/joint_layout_builder.hpp>
#include <rexlib/core/layout/strided_layout.hpp>

#include <stdexcept>
#include <utility>
#include <vector>

namespace rexlib
{
namespace cpu
{

template <typename ReductionIndexing>
batch_reduce_map_plan::batch_reduce_map_plan(
	batched_map_plan map_plan,
	span<const operand_descriptor> input_descriptors,
	span<const std::size_t> core_inputs,
	std::size_t statistic_count,
	ReductionIndexing reduction_indexing
)
	: m_map_plan(std::move(map_plan))
{
	if (core_inputs.empty())
	{
		throw std::invalid_argument(
			"batch_reduce_map_plan: No input operand carries the core of the "
			"outputs."
		);
	}

	const auto &map_frame = m_map_plan.get_frame();
	const auto core_extents = map_frame.get_core_extents();

	joint_layout_builder scalar_builder;
	joint_layout_builder reduced_builder;
	scalar_builder.set_extents(span<const std::size_t>());
	reduced_builder.set_extents(make_span(core_extents));

	std::vector<std::ptrdiff_t> strides;
	for (const auto input : core_inputs)
	{
		const auto aligned = map_frame.align(
			input_descriptors[input].get_layout(),
			true
		);
		aligned.get_strides(strides);
		const auto core_strides = map_frame.select_core(make_span(strides));
		reduced_builder.add_operand(
			make_span(core_extents),
			make_span(core_strides),
			0
		);
		scalar_builder.add_operand(
			span<const std::size_t>(),
			span<const std::ptrdiff_t>(),
			0
		);
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

	m_scalar_layout = scalar_builder.build();
	m_reduced_layout = reduced_builder.build();
}

const batched_map_plan& batch_reduce_map_plan::get_map_plan() const noexcept
{
	return m_map_plan;
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

#define REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN(reduction) \
	template batch_reduce_map_plan::batch_reduce_map_plan( \
		batched_map_plan, \
		span<const operand_descriptor>, \
		span<const std::size_t>, \
		std::size_t, \
		reduction \
	)

REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN(no_index_tag);
REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN(linear_index_tag);
REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN(multidimensional_index_tag);

#undef REXLIB_INSTANTIATE_BATCH_REDUCE_MAP_PLAN

} // namespace cpu
} // namespace rexlib
