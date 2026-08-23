// SPDX-License-Identifier: GPL-3.0-only

#include "linalg_program_builder.hpp"

#include <xmipp4/core/dispatch/operand_signature.hpp>
#include <xmipp4/core/layout/strided_layout.hpp>
#include <xmipp4/core/meta/type_list.hpp>
#include <xmipp4/core/platform/cpp_attributes.hpp>

#include <backends/cpu/loops/elementwise_loop.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/loops/parallel_grain.hpp>

#include <tuple>
#include <utility>

namespace xmipp4
{
namespace cpu
{

/**
 * @brief Adapts a per-batch-element kernel to the functor_program interface.
 *
 * Stores the kernel and the planned batch/core split, and on invocation
 * drives run_elementwise_loop over the batch, handing the kernel one output
 * pointer and two input pointers per batch element.
 */
template <typename F, typename Outputs, typename Inputs>
class linalg_loop_functor;

template <typename F, typename Out, typename Left, typename Right>
class linalg_loop_functor<F, type_list<Out>, type_list<Left, Right>>
{
public:
	linalg_loop_functor(F functor, linalg_core_layout_plan plan)
		: m_functor(std::move(functor))
		, m_plan(std::move(plan))
	{
	}

	void operator()(
		std::tuple<Out*> outputs,
		std::tuple<const Left*, const Right*> inputs,
		std::tuple<>,
		thread_pool &pool
	) const
	{
		// One iteration here is a whole matrix product, so the grain is
		// stated in those rather than in elements: anything of a realistic
		// size makes it one batch element per thread.
		run_elementwise_loop(
			m_functor,
			m_plan.get_batch_layout(),
			loop_schedule(pool, grain_for_cost(m_plan.get_core_cost())),
			std::get<0>(outputs),
			std::get<0>(inputs),
			std::get<1>(inputs)
		);
	}

private:
	XMIPP4_NO_UNIQUE_ADDRESS F m_functor;
	linalg_core_layout_plan m_plan;
};

template <typename Op, typename KernelFactory, typename TypeDispatcher>
bool
linalg_program_builder<Op, KernelFactory, TypeDispatcher>
::accepts_signatures(
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures
) const noexcept
{
	using core_ranks = linalg_core_ranks<Op>;

	return
		output_signatures[0].get_layout().get_rank() >= core_ranks::output &&
		input_signatures[0].get_layout().get_rank() >= core_ranks::left &&
		input_signatures[1].get_layout().get_rank() >= core_ranks::right;
}

template <typename Op, typename KernelFactory, typename TypeDispatcher>
linalg_core_layout_plan
linalg_program_builder<Op, KernelFactory, TypeDispatcher>::make_plan(
	const Op& /*operation*/,
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures
) const
{
	using core_ranks = linalg_core_ranks<Op>;

	return linalg_core_layout_plan::for_trailing_core(
		output_signatures,
		input_signatures,
		core_ranks::output,
		core_ranks::left,
		core_ranks::right
	);
}

template <typename Op, typename KernelFactory, typename TypeDispatcher>
template <typename... Outs, typename... Ins>
auto
linalg_program_builder<Op, KernelFactory, TypeDispatcher>::make_loop_functor(
	const Op &operation,
	linalg_core_layout_plan &plan,
	type_list<Outs...> output_element_types,
	type_list<Ins...> input_element_types
) const
{
	auto kernel = m_kernel_factory(
		operation,
		output_element_types,
		input_element_types,
		plan
	);

	return linalg_loop_functor<
		decltype(kernel),
		type_list<Outs...>,
		type_list<Ins...>
	>(std::move(kernel), std::move(plan));
}

} // namespace cpu
} // namespace xmipp4
