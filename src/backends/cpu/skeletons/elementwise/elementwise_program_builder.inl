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

#include <backends/cpu/loops/batch_loop.hpp>
#include <backends/cpu/loops/elementwise_loop.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/loops/parallel_grain.hpp>
#include <backends/cpu/loops/layouts/index_operands.hpp>
#include <backends/cpu/loops/layouts/named_core_frame.hpp>
#include <backends/cpu/skeletons/index_array.hpp>
#include <backends/cpu/skeletons/output_core_frame.hpp>
#include <backends/cpu/skeletons/elementwise/batch_pointers.hpp>

#include <cstddef>
#include <tuple>
#include <type_traits>
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

namespace detail
{

template <std::size_t Index, typename Indices>
struct contains_index;

template <std::size_t Index, std::size_t... Indices>
struct contains_index<Index, std::index_sequence<Indices...>>
	: std::integral_constant<
		bool,
		!std::is_same<
			std::integer_sequence<bool, (Index == Indices)...>,
			std::integer_sequence<bool, (Indices != Indices)...>
		>::value
	>
{
};

template <
	std::size_t Count,
	typename Excluded,
	typename Selected = std::index_sequence<>,
	std::size_t Index = 0,
	bool Done = (Index == Count)
>
struct complement_indices;

template <
	std::size_t Count,
	typename Excluded,
	std::size_t... Selected,
	std::size_t Index
>
struct complement_indices<
	Count,
	Excluded,
	std::index_sequence<Selected...>,
	Index,
	true
>
{
	using type = std::index_sequence<Selected...>;
};

template <
	std::size_t Count,
	typename Excluded,
	std::size_t... Selected,
	std::size_t Index
>
struct complement_indices<
	Count,
	Excluded,
	std::index_sequence<Selected...>,
	Index,
	false
>
	: complement_indices<
		Count,
		Excluded,
		typename std::conditional<
			contains_index<Index, Excluded>::value,
			std::index_sequence<Selected...>,
			std::index_sequence<Selected..., Index>
		>::type,
		Index + 1
	>
{
};

} // namespace detail

/**
 * @brief Adapts the kernel maker of an elementwise operation with inputs of
 * their own core to the functor_program interface.
 *
 * Walks the batch layout, makes the kernel of every batch element from the
 * inputs placed there, and maps its core.
 */
template <
	typename Maker,
	typename Indexing,
	typename Outputs,
	typename Inputs,
	typename MappedInputs
>
class batched_elementwise_loop_functor;

template <
	typename Maker,
	typename Indexing,
	typename... Outs,
	typename... Ins,
	std::size_t... MappedInputs
>
class batched_elementwise_loop_functor<
	Maker,
	Indexing,
	type_list<Outs...>,
	type_list<Ins...>,
	std::index_sequence<MappedInputs...>
>
{
public:
	using output_pointers_type = std::tuple<Outs*...>;
	using input_pointers_type = std::tuple<const Ins*...>;

	batched_elementwise_loop_functor(Maker maker, batched_map_plan plan)
		: m_maker(std::move(maker))
		, m_plan(std::move(plan))
	{
	}

	void operator()(
		output_pointers_type outputs,
		input_pointers_type inputs,
		std::tuple<>,
		thread_pool &pool
	) const
	{
		const auto core_count = m_plan.get_core_count();
		if (m_plan.get_batch_count() == 0 || core_count == 0)
		{
			return;
		}

		run_batch(
			loop_schedule(pool, 1),
			core_count,
			outputs,
			inputs,
			std::index_sequence_for<Outs...>(),
			std::index_sequence_for<Ins...>()
		);
	}

private:
	template <std::size_t... OutputIndices, std::size_t... InputIndices>
	void run_batch(
		const loop_schedule &everywhere,
		std::size_t core_count,
		const output_pointers_type &outputs,
		const input_pointers_type &inputs,
		std::index_sequence<OutputIndices...>,
		std::index_sequence<InputIndices...>
	) const
	{
		run_batch_loop(
			[this] (const loop_schedule &inner, auto... pointers)
			{
				this->visit(inner, std::make_tuple(pointers...));
			},
			m_plan.get_batch_layout(),
			batch_indexing_of<Indexing>(),
			everywhere,
			core_count,
			core_count,
			std::get<InputIndices>(inputs)...,
			std::get<OutputIndices>(outputs)...
		);
	}

	template <typename Pointers>
	void visit(const loop_schedule &inner, const Pointers &pointers) const
	{
		const auto inputs = slice_pointers<0>(
			pointers,
			std::index_sequence_for<Ins...>()
		);
		const auto outputs = slice_pointers<sizeof...(Ins)>(
			pointers,
			std::index_sequence_for<Outs...>()
		);

		map(
			inner,
			make_map(
				inputs,
				get_batch_counter(pointers, Indexing()),
				Indexing()
			),
			outputs,
			inputs,
			std::index_sequence_for<Outs...>()
		);
	}

	template <typename IndexTag>
	auto make_map(
		const input_pointers_type &inputs,
		std::size_t /*counter*/,
		IndexTag
	) const
	{
		return m_maker.make_map(inputs);
	}

	auto make_map(
		const input_pointers_type &inputs,
		std::size_t counter,
		linear_index_tag
	) const
	{
		return m_maker.make_map(inputs, counter);
	}

	template <typename Kernel, std::size_t... OutputIndices>
	void map(
		const loop_schedule &inner,
		const Kernel &kernel,
		const output_pointers_type &outputs,
		const input_pointers_type &inputs,
		std::index_sequence<OutputIndices...>
	) const
	{
		run_indexed_elementwise_loop(
			kernel,
			m_plan.get_map_layout(),
			Indexing(),
			inner.with_grain(grain_for_cost(1)),
			std::get<OutputIndices>(outputs)...,
			std::get<MappedInputs>(inputs)...
		);
	}

	REXLIB_NO_UNIQUE_ADDRESS Maker m_maker;
	batched_map_plan m_plan;
};

template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatch,
	typename Indexing
>
auto
elementwise_program_builder<Op, KernelFactory, TypeDispatch, Indexing>
::make_plan(
	const Op &operation,
	span<const operand_descriptor> output_descriptors,
	span<const operand_descriptor> input_descriptors
) const
{
	return make_plan(
		operation,
		output_descriptors,
		input_descriptors,
		batched_type()
	);
}

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
	span<const operand_descriptor> input_descriptors,
	std::false_type /*batched*/
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
batched_map_plan
elementwise_program_builder<Op, KernelFactory, TypeDispatch, Indexing>
::make_plan(
	const Op &operation,
	span<const operand_descriptor> output_descriptors,
	span<const operand_descriptor> input_descriptors,
	std::true_type /*batched*/
) const
{
	const auto mapped_inputs = make_index_array(
		typename detail::complement_indices<
			Op::input_operand_count,
			own_core_inputs_type
		>::type()
	);

	return batched_map_plan(
		make_output_core_frame(operation, output_descriptors),
		output_descriptors,
		input_descriptors,
		shape_rule_descriptor_of<typename Op::shape_rule_type>::get()
			.get_input_cores(),
		make_span(mapped_inputs),
		Indexing()
	);
}

template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatch,
	typename Indexing
>
template <typename Plan, typename... Outs, typename... Ins>
auto
elementwise_program_builder<Op, KernelFactory, TypeDispatch, Indexing>
::make_loop_functor(
	const Op &operation,
	Plan &plan,
	type_list<Outs...> output_element_types,
	type_list<Ins...> input_element_types
) const
{
	return make_loop_functor(
		operation,
		plan,
		output_element_types,
		input_element_types,
		batched_type()
	);
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
	type_list<Ins...> input_element_types,
	std::false_type /*batched*/
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
template <typename... Outs, typename... Ins>
auto
elementwise_program_builder<Op, KernelFactory, TypeDispatch, Indexing>
::make_loop_functor(
	const Op &operation,
	batched_map_plan &plan,
	type_list<Outs...> output_element_types,
	type_list<Ins...> input_element_types,
	std::true_type /*batched*/
) const
{
	auto maker = m_kernel_factory(
		operation,
		output_element_types,
		input_element_types,
		static_cast<const batched_map_plan&>(plan)
	);

	return batched_elementwise_loop_functor<
		decltype(maker),
		Indexing,
		type_list<Outs...>,
		type_list<Ins...>,
		typename detail::complement_indices<
			sizeof...(Ins),
			own_core_inputs_type
		>::type
	>(std::move(maker), std::move(plan));
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
