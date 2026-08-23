// SPDX-License-Identifier: GPL-3.0-only

#include "sequence_program_builder.hpp"

#include <xmipp4/core/dispatch/operand_signature.hpp>
#include <xmipp4/core/meta/type_list.hpp>
#include <xmipp4/core/platform/cpp_attributes.hpp>

#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/loops/parallel_grain.hpp>
#include <backends/cpu/loops/sequence_loop.hpp>

#include <tuple>
#include <utility>

namespace xmipp4
{
namespace cpu
{

/**
 * @brief Adapts a sequence generator to the functor_program interface.
 *
 * Stores the generator and the planned traversal, and on invocation drives
 * run_sequence_loop with them.
 */
template <typename F, typename Outputs, typename Inputs>
class sequence_loop_functor;

template <typename F, typename Out>
class sequence_loop_functor<F, type_list<Out>, type_list<>>
{
public:
	sequence_loop_functor(F functor, sequence_layout_plan plan)
		: m_functor(std::move(functor))
		, m_plan(plan)
	{
	}

	void operator()(
		std::tuple<Out*> outputs,
		std::tuple<>,
		std::tuple<>,
		thread_pool &pool
	) const
	{
		run_sequence_loop(
			m_functor,
			std::get<0>(outputs) + m_plan.get_offset(),
			m_plan.get_count(),
			m_plan.get_stride(),
			loop_schedule(pool, grain_for_cost(1))
		);
	}

private:
	XMIPP4_NO_UNIQUE_ADDRESS F m_functor;
	sequence_layout_plan m_plan;
};

template <typename Op, typename KernelFactory, typename TypeDispatcher>
bool
sequence_program_builder<Op, KernelFactory, TypeDispatcher>
::accepts_signatures(
	span<const operand_signature> output_signatures,
	span<const operand_signature> /*input_signatures*/
) const noexcept
{
	return output_signatures[0].get_layout().get_rank() == 1;
}

template <typename Op, typename KernelFactory, typename TypeDispatcher>
sequence_layout_plan
sequence_program_builder<Op, KernelFactory, TypeDispatcher>::make_plan(
	const Op& /*operation*/,
	span<const operand_signature> output_signatures,
	span<const operand_signature> /*input_signatures*/
) const
{
	return sequence_layout_plan(output_signatures[0].get_layout());
}

template <typename Op, typename KernelFactory, typename TypeDispatcher>
template <typename... Outs, typename... Ins>
auto
sequence_program_builder<Op, KernelFactory, TypeDispatcher>
::make_loop_functor(
	const Op &operation,
	sequence_layout_plan &plan,
	type_list<Outs...> output_element_types,
	type_list<Ins...> input_element_types
) const
{
	auto kernel = m_kernel_factory(
		operation,
		output_element_types,
		input_element_types
	);

	return sequence_loop_functor<
		decltype(kernel),
		type_list<Outs...>,
		type_list<Ins...>
	>(std::move(kernel), plan);
}

} // namespace cpu
} // namespace xmipp4
