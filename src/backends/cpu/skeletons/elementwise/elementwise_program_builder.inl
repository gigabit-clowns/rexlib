// SPDX-License-Identifier: GPL-3.0-only

#include "elementwise_program_builder.hpp"

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/joint_layout_builder.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/operation/shape_rule/core_placement.hpp>
#include <rexlib/core/operation/shape_rule/shape_rule_descriptor_of.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/core/platform/cpp_attributes.hpp>

#include <backends/cpu/loops/elementwise_loop.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/loops/parallel_grain.hpp>
#include <backends/cpu/loops/layouts/index_operands.hpp>
#include <backends/cpu/loops/layouts/named_core_frame.hpp>

#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Adapts a per-element kernel to the functor_program interface.
 *
 * Stores the kernel and the joint layout, and on invocation unpacks the
 * operand pointer tuples and drives run_indexed_elementwise_loop, passing the
 * output pointers followed by the input pointers, and the index @p Indexing
 * asks for, to the kernel.
 */
template <typename F, typename Indexing, typename Outputs, typename Inputs>
class elementwise_loop_functor;

template <typename F, typename Indexing, typename... Outs, typename... Ins>
class elementwise_loop_functor<
	F,
	Indexing,
	type_list<Outs...>,
	type_list<Ins...>
>
{
public:
	elementwise_loop_functor(F functor, joint_layout layout)
		: m_functor(std::move(functor))
		, m_layout(std::move(layout))
	{
	}

	void operator()(
		std::tuple<Outs*...> outputs,
		std::tuple<const Ins*...> inputs,
		std::tuple<>,
		thread_pool &pool
	) const
	{
		run(
			outputs,
			inputs,
			loop_schedule(pool, grain_for_cost(1)),
			std::index_sequence_for<Outs...>(),
			std::index_sequence_for<Ins...>()
		);
	}

private:
	template <std::size_t... OutputIndices, std::size_t... InputIndices>
	void run(
		const std::tuple<Outs*...> &outputs,
		const std::tuple<const Ins*...> &inputs,
		const loop_schedule &schedule,
		std::index_sequence<OutputIndices...>,
		std::index_sequence<InputIndices...>
	) const
	{
		run_indexed_elementwise_loop(
			m_functor,
			m_layout,
			Indexing(),
			schedule,
			std::get<OutputIndices>(outputs)...,
			std::get<InputIndices>(inputs)...
		);
	}

	REXLIB_NO_UNIQUE_ADDRESS F m_functor;
	joint_layout m_layout;
};

template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatch,
	typename Indexing
>
joint_layout
elementwise_program_builder<Op, KernelFactory, TypeDispatch, Indexing>
::make_plan(
	const Op& /*operation*/,
	span<const operand_descriptor> output_descriptors,
	span<const operand_descriptor> input_descriptors
) const
{
	const auto &rule =
		shape_rule_descriptor_of<typename Op::shape_rule_type>::get();
	REXLIB_ASSERT(rule.get_placement() == core_placement::trailing);

	const auto output_cores = rule.get_output_cores();
	const auto &output_core = output_cores[0];
	REXLIB_ASSERT(!output_core.has_run());

	std::vector<std::size_t> frame_extents;
	output_descriptors[0].get_layout().get_extents(frame_extents);
	const auto frame = make_trailing_core_frame(
		std::move(frame_extents),
		output_core.get_slots().size()
	);

	joint_layout_builder layout_builder;
	for (const auto &descriptor : output_descriptors)
	{
		layout_builder.add_operand(descriptor.get_layout());
	}
	const auto input_cores = rule.get_input_cores();
	for (std::size_t i = 0; i < input_descriptors.size(); ++i)
	{
		layout_builder.add_operand(
			frame.align(
				input_descriptors[i].get_layout(),
				!input_cores[i].is_empty()
			)
		);
	}
	append_index_operands(layout_builder, output_descriptors, Indexing());

	return layout_builder.build();
}

template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatch,
	typename Indexing
>
template <typename... Outs, typename... Ins>
auto
elementwise_program_builder<Op, KernelFactory, TypeDispatch, Indexing>
::make_loop_functor(
	const Op &operation,
	joint_layout &layout,
	type_list<Outs...> output_element_types,
	type_list<Ins...> input_element_types
) const
{
	auto kernel = m_kernel_factory(
		operation,
		output_element_types,
		input_element_types
	);

	return elementwise_loop_functor<
		decltype(kernel),
		Indexing,
		type_list<Outs...>,
		type_list<Ins...>
	>(std::move(kernel), std::move(layout));
}

template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatch,
	typename Indexing
>
void
elementwise_program_builder<Op, KernelFactory, TypeDispatch, Indexing>
::append_index_operands(
	joint_layout_builder& /*layout_builder*/,
	span<const operand_descriptor> /*output_descriptors*/,
	no_index_tag
) const noexcept
{
}

template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatch,
	typename Indexing
>
template <typename IndexTag>
void
elementwise_program_builder<Op, KernelFactory, TypeDispatch, Indexing>
::append_index_operands(
	joint_layout_builder &layout_builder,
	span<const operand_descriptor> output_descriptors,
	IndexTag indexing
) const
{
	std::vector<std::size_t> extents;
	output_descriptors[0].get_layout().get_extents(extents);
	add_index_operands(layout_builder, make_span(extents), indexing);
}

} // namespace cpu
} // namespace rexlib
