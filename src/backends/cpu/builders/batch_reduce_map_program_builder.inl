// SPDX-License-Identifier: GPL-3.0-only

#include "batch_reduce_map_program_builder.hpp"

#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/core/operation/shape_rule/shape_rule_descriptor_of.hpp>

#include <backends/cpu/builders/core_input_indices.hpp>
#include <backends/cpu/loops/elementwise_loop.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/loops/parallel_grain.hpp>
#include <backends/cpu/loops/reduction_loop.hpp>

#include <array>
#include <cstddef>
#include <tuple>
#include <utility>

namespace rexlib
{
namespace cpu
{

namespace detail
{

template <std::size_t Offset, typename Tuple, std::size_t... Indices>
auto slice_tuple(const Tuple &tuple, std::index_sequence<Indices...>)
{
	return std::make_tuple(std::get<Offset + Indices>(tuple)...);
}

template <typename Pointers>
std::size_t batch_counter_of(const Pointers&, no_index_tag) noexcept
{
	return 0;
}

template <typename Pointers>
std::size_t batch_counter_of(
	const Pointers &pointers,
	linear_index_tag
) noexcept
{
	return std::get<std::tuple_size<Pointers>::value - 1>(pointers);
}

} // namespace detail

/**
 * @brief Adapts a batch reduce-map kernel to the functor_program interface.
 *
 * Walks the batch layout, and for every batch element runs each reduction
 * over its core, then the map.
 */
template <
	typename Kernel,
	std::size_t StatisticCount,
	std::size_t PassCount,
	typename ReductionIndexing,
	typename MapIndexing,
	typename Outputs,
	typename Inputs,
	typename CoreInputs
>
class batch_reduce_map_loop_functor;

template <
	typename Kernel,
	std::size_t StatisticCount,
	std::size_t PassCount,
	typename ReductionIndexing,
	typename MapIndexing,
	typename... Outs,
	typename... Ins,
	std::size_t... CoreInputs
>
class batch_reduce_map_loop_functor<
	Kernel,
	StatisticCount,
	PassCount,
	ReductionIndexing,
	MapIndexing,
	type_list<Outs...>,
	type_list<Ins...>,
	std::index_sequence<CoreInputs...>
>
{
public:
	using statistics_type = std::array<double, StatisticCount>;
	using output_pointers_type = std::tuple<Outs*...>;
	using input_pointers_type = std::tuple<const Ins*...>;

	batch_reduce_map_loop_functor(Kernel kernel, batch_reduce_map_plan plan)
		: m_kernel(std::move(kernel))
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
		const auto batch_count = m_plan.get_batch_count();
		const auto core_count = m_plan.get_core_count();
		if (batch_count == 0 || core_count == 0)
		{
			return;
		}

		const loop_schedule everywhere(pool, 1);
		const auto inside = should_split_inner_space(
			batch_count,
			core_count,
			everywhere.get_concurrency()
		);
		const auto outer = inside
			? loop_schedule()
			: everywhere.with_grain(grain_for_cost((PassCount + 1)*core_count));
		const auto inner = inside ? everywhere : loop_schedule();

		run_batch(
			outer,
			inner,
			outputs,
			inputs,
			std::index_sequence_for<Outs...>(),
			std::index_sequence_for<Ins...>()
		);
	}

private:
	template <std::size_t... OutputIndices, std::size_t... InputIndices>
	void run_batch(
		const loop_schedule &outer,
		const loop_schedule &inner,
		const output_pointers_type &outputs,
		const input_pointers_type &inputs,
		std::index_sequence<OutputIndices...>,
		std::index_sequence<InputIndices...>
	) const
	{
		run_indexed_elementwise_loop(
			[this, &inner] (auto... pointers)
			{
				this->visit(inner, std::make_tuple(pointers...));
			},
			m_plan.get_batch_layout(),
			MapIndexing(),
			outer,
			std::get<InputIndices>(inputs)...,
			std::get<OutputIndices>(outputs)...
		);
	}

	template <typename Pointers>
	void visit(const loop_schedule &inner, const Pointers &pointers) const
	{
		process(
			inner,
			detail::slice_tuple<0>(
				pointers,
				std::index_sequence_for<Ins...>()
			),
			detail::slice_tuple<sizeof...(Ins)>(
				pointers,
				std::index_sequence_for<Outs...>()
			),
			detail::batch_counter_of(pointers, MapIndexing())
		);
	}

	void process(
		const loop_schedule &inner,
		const input_pointers_type &inputs,
		const output_pointers_type &outputs,
		std::size_t counter
	) const
	{
		statistics_type statistics = {};
		for (std::size_t pass = 0; pass < PassCount; ++pass)
		{
			statistics = reduce(inner, pass, statistics, inputs);
		}

		map(
			inner,
			make_map(statistics, inputs, counter, MapIndexing()),
			outputs,
			inputs,
			std::index_sequence_for<Outs...>()
		);
	}

	statistics_type reduce(
		const loop_schedule &inner,
		std::size_t pass,
		const statistics_type &previous,
		const input_pointers_type &inputs
	) const
	{
		statistics_type result = {};
		run_indexed_reduction_loop(
			m_kernel.make_reduction(pass, previous),
			m_plan.get_scalar_layout(),
			m_plan.get_reduced_layout(),
			m_plan.get_core_count(),
			point_to(result, std::make_index_sequence<StatisticCount>()),
			std::make_tuple(std::get<CoreInputs>(inputs)...),
			ReductionIndexing(),
			inner
		);
		return result;
	}

	template <std::size_t... Indices>
	static auto point_to(
		statistics_type &statistics,
		std::index_sequence<Indices...>
	) noexcept
	{
		return std::make_tuple(&statistics[Indices]...);
	}

	auto make_map(
		const statistics_type &statistics,
		const input_pointers_type &inputs,
		std::size_t /*counter*/,
		no_index_tag
	) const
	{
		return m_kernel.make_map(statistics, inputs);
	}

	auto make_map(
		const statistics_type &statistics,
		const input_pointers_type &inputs,
		std::size_t counter,
		linear_index_tag
	) const
	{
		return m_kernel.make_map(statistics, inputs, counter);
	}

	template <typename MapKernel, std::size_t... OutputIndices>
	void map(
		const loop_schedule &inner,
		const MapKernel &kernel,
		const output_pointers_type &outputs,
		const input_pointers_type &inputs,
		std::index_sequence<OutputIndices...>
	) const
	{
		run_indexed_elementwise_vector_loop(
			kernel,
			m_plan.get_map_layout(),
			MapIndexing(),
			inner.with_grain(grain_for_cost(1)),
			std::get<OutputIndices>(outputs)...,
			std::get<CoreInputs>(inputs)...
		);
	}

	REXLIB_NO_UNIQUE_ADDRESS Kernel m_kernel;
	batch_reduce_map_plan m_plan;
};

template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatcher,
	typename ReductionIndexing,
	typename MapIndexing
>
batch_reduce_map_plan
batch_reduce_map_program_builder<
	Op,
	KernelFactory,
	TypeDispatcher,
	ReductionIndexing,
	MapIndexing
>::make_plan(
	const Op &operation,
	span<const operand_descriptor> output_descriptors,
	span<const operand_descriptor> input_descriptors
) const
{
	using shape_rule_type = typename Op::shape_rule_type;

	return batch_reduce_map_plan(
		output_descriptors,
		input_descriptors,
		shape_rule_descriptor_of<shape_rule_type>::get().get_input_cores(),
		operation.get_parameters().get_axes(),
		KernelFactory::statistic_count,
		ReductionIndexing(),
		MapIndexing()
	);
}

template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatcher,
	typename ReductionIndexing,
	typename MapIndexing
>
template <typename... Outs, typename... Ins>
auto
batch_reduce_map_program_builder<
	Op,
	KernelFactory,
	TypeDispatcher,
	ReductionIndexing,
	MapIndexing
>::make_loop_functor(
	const Op &operation,
	batch_reduce_map_plan &plan,
	type_list<Outs...> output_element_types,
	type_list<Ins...> input_element_types
) const
{
	auto kernel = m_kernel_factory(
		operation,
		output_element_types,
		input_element_types,
		static_cast<const batch_reduce_map_plan&>(plan)
	);

	return batch_reduce_map_loop_functor<
		decltype(kernel),
		KernelFactory::statistic_count,
		KernelFactory::pass_count,
		ReductionIndexing,
		MapIndexing,
		type_list<Outs...>,
		type_list<Ins...>,
		typename core_input_indices<typename Op::shape_rule_type>::type
	>(std::move(kernel), std::move(plan));
}

} // namespace cpu
} // namespace rexlib
