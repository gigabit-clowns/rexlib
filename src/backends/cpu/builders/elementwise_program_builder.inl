// SPDX-License-Identifier: GPL-3.0-only

#include "elementwise_program_builder.hpp"

#include <xmipp4/core/dispatch/operand_signature.hpp>
#include <xmipp4/core/layout/joint_layout_builder.hpp>
#include <xmipp4/core/meta/type_list.hpp>
#include <xmipp4/core/platform/cpp_attributes.hpp>

#include <backends/cpu/loops/elementwise_loop.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/loops/parallel_grain.hpp>

#include <cstddef>
#include <tuple>
#include <utility>

namespace xmipp4
{
namespace cpu
{

/**
 * @brief Adapts a per-element kernel to the functor_program interface.
 *
 * Stores the kernel and the joint layout, and on invocation unpacks the
 * operand pointer tuples and drives run_elementwise_loop, passing the output
 * pointers followed by the input pointers to the kernel.
 */
template <typename F, typename Outputs, typename Inputs>
class elementwise_loop_functor;

template <typename F, typename... Outs, typename... Ins>
class elementwise_loop_functor<F, type_list<Outs...>, type_list<Ins...>>
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
		run_elementwise_loop(
			m_functor,
			m_layout,
			schedule,
			std::get<OutputIndices>(outputs)...,
			std::get<InputIndices>(inputs)...
		);
	}

	XMIPP4_NO_UNIQUE_ADDRESS F m_functor;
	joint_layout m_layout;
};

template <typename Op, typename KernelFactory, typename TypeDispatcher>
joint_layout
elementwise_program_builder<Op, KernelFactory, TypeDispatcher>::make_plan(
	const Op& /*operation*/,
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures
) const
{
	joint_layout_builder layout_builder;
	for (const auto &signature : output_signatures)
	{
		layout_builder.add_operand(signature.get_layout());
	}
	for (const auto &signature : input_signatures)
	{
		layout_builder.add_operand(signature.get_layout());
	}

	return layout_builder.build();
}

template <typename Op, typename KernelFactory, typename TypeDispatcher>
template <typename... Outs, typename... Ins>
auto
elementwise_program_builder<Op, KernelFactory, TypeDispatcher>
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
		type_list<Outs...>,
		type_list<Ins...>
	>(std::move(kernel), std::move(layout));
}

} // namespace cpu
} // namespace xmipp4
