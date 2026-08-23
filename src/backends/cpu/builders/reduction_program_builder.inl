// SPDX-License-Identifier: GPL-3.0-only

#include "reduction_program_builder.hpp"

#include <xmipp4/core/dispatch/operand_signature.hpp>
#include <xmipp4/core/meta/type_list.hpp>
#include <xmipp4/core/platform/cpp_attributes.hpp>

#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/loops/parallel_grain.hpp>
#include <backends/cpu/loops/reduction_loop.hpp>

#include <tuple>
#include <utility>

namespace rex
{
namespace cpu
{

/**
 * @brief Adapts a reduction kernel to the functor_program interface.
 *
 * Stores the kernel and the planned iteration, and on invocation drives
 * run_reduction_loop with them.
 */
template <typename F, typename Outputs, typename Inputs>
class reduction_loop_functor;

template <typename F, typename... Outs, typename... Ins>
class reduction_loop_functor<F, type_list<Outs...>, type_list<Ins...>>
{
public:
	reduction_loop_functor(F functor, reduction_layout_plan plan)
		: m_functor(std::move(functor))
		, m_plan(std::move(plan))
	{
	}

	void operator()(
		std::tuple<Outs*...> outputs,
		std::tuple<const Ins*...> inputs,
		std::tuple<>,
		thread_pool &pool
	) const
	{
		// The grain is settled inside the loop, which is the only place that
		// knows how deep the fold behind each output is.
		run_reduction_loop(
			m_functor,
			m_plan.get_kept_layout(),
			m_plan.get_reduced_layout(),
			m_plan.get_reduction_count(),
			outputs,
			inputs,
			loop_schedule(pool, 1)
		);
	}

private:
	REX_NO_UNIQUE_ADDRESS F m_functor;
	reduction_layout_plan m_plan;
};

template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatcher,
	bool Ordered
>
reduction_layout_plan
reduction_program_builder<Op, KernelFactory, TypeDispatcher, Ordered>
::make_plan(
	const Op &operation,
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures
) const
{
	const auto &shape_policy = operation.get_shape_policy();
	return reduction_layout_plan(
		output_signatures,
		input_signatures,
		shape_policy.get_axes(),
		shape_policy.get_keep_dimensions(),
		Ordered
	);
}

template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatcher,
	bool Ordered
>
template <typename... Outs, typename... Ins>
auto
reduction_program_builder<Op, KernelFactory, TypeDispatcher, Ordered>
::make_loop_functor(
	const Op &operation,
	reduction_layout_plan &plan,
	type_list<Outs...> output_element_types,
	type_list<Ins...> input_element_types
) const
{
	auto kernel = m_kernel_factory(
		operation,
		output_element_types,
		input_element_types
	);

	return reduction_loop_functor<
		decltype(kernel),
		type_list<Outs...>,
		type_list<Ins...>
	>(std::move(kernel), std::move(plan));
}

} // namespace cpu
} // namespace rex
