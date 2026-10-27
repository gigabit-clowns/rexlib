// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/operation/shape_rule/core_placement.hpp>
#include <rexlib/core/platform/cpp_attributes.hpp>
#include <rexlib/core/span.hpp>

#include <backends/cpu/builders/dispatched_program_builder.hpp>
#include <backends/cpu/builders/program_builder_registration_macros.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>
#include <backends/cpu/loops/element_index_tags.hpp>
#include <backends/cpu/plans/batch_reduce_map_plan.hpp>

namespace rexlib
{
namespace cpu
{

/**
 * @brief CPU program builder for operations that reduce the core of every
 * batch element to statistics, and then map that core with them.
 *
 * The core axes are the ones the operation names. Each batch element is
 * taken by one thread at a time, which runs every reduction over its core
 * and then the map, so that the core is read from memory once and found in
 * cache by the passes after the first. When there are too few batch
 * elements to occupy the threads, the batch elements are taken one after
 * another instead, and every pass is spread over the threads.
 *
 * The kernel factory declares, as static constants, `statistic_count`, the
 * number of statistics a reduction produces, and `pass_count`, the number
 * of reductions, at least one. Called as
 * `factory(operation, output_types, input_types, plan)`, it returns a kernel
 * providing:
 * @code
 * reduction_kernel make_reduction(
 *     std::size_t pass,
 *     const std::array<double, statistic_count> &previous
 * ) const;
 * map_kernel make_map(
 *     const std::array<double, statistic_count> &statistics,
 *     const std::tuple<const Ins*...> &inputs
 *     [, std::size_t counter]
 * ) const;
 * @endcode
 * A reduction kernel is an element reduction kernel, as
 * @ref run_reduction_loop describes, whose inputs are the inputs carrying
 * the core axes and whose outputs are the statistics, of type double. The
 * first reduction is handed statistics of zero. A map kernel is a vector
 * kernel, as @ref run_elementwise_vector_loop describes, over the outputs
 * followed by the inputs carrying the core axes. It is made for each batch
 * element from the statistics of the last reduction and from the inputs
 * placed at that batch element, so that it reads the inputs without core
 * axes from there.
 *
 * The plan moves into the program once the kernel is made, so the kernel
 * copies what it needs of it rather than referring to it.
 *
 * @tparam Op The operation. Its shape rule names its core axes.
 * @tparam KernelFactory The kernel factory.
 * @tparam TypeDispatcher The type dispatcher.
 * @tparam ReductionIndexing The index handed to the reduction kernels,
 * over the core axes in ascending order.
 * @tparam MapIndexing no_index_tag, or linear_index_tag to hand the map the
 * counter of @ref batch_reduce_map_plan: its batch part to make_map, and
 * its core part to the map kernel.
 */
template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatcher = rule_type_dispatcher<typename Op::type_rule_type>,
	typename ReductionIndexing = no_index_tag,
	typename MapIndexing = no_index_tag
>
class batch_reduce_map_program_builder final
	: public dispatched_program_builder<
		batch_reduce_map_program_builder<
			Op,
			KernelFactory,
			TypeDispatcher,
			ReductionIndexing,
			MapIndexing
		>,
		Op,
		TypeDispatcher
	>
{
	static_assert(
		Op::shape_rule_type::placement_type::placement ==
			core_placement::named,
		"A batch reduce-map builder walks the axes the operation names, so "
		"the operation's shape rule must name its core axes."
	);

	static_assert(
		KernelFactory::pass_count > 0,
		"A batch reduce-map builder needs at least one reduction."
	);

public:
	batch_reduce_map_program_builder() noexcept = default;

	batch_reduce_map_plan make_plan(
		const Op &operation,
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors
	) const;

	template <typename... Outs, typename... Ins>
	auto make_loop_functor(
		const Op &operation,
		batch_reduce_map_plan &plan,
		type_list<Outs...> output_element_types,
		type_list<Ins...> input_element_types
	) const;

private:
	using kernel_factory_type = KernelFactory;

	REXLIB_NO_UNIQUE_ADDRESS kernel_factory_type m_kernel_factory;
};

} // namespace cpu
} // namespace rexlib

/**
 * @brief Register a CPU batch reduce-map builder for an operation.
 *
 * @param name Identifier of the registration object.
 * @param op The operation.
 * @param kernel_factory The kernel factory.
 */
#define REXLIB_REGISTER_BATCH_REDUCE_MAP_PROGRAM_BUILDER( \
	name, op, kernel_factory \
) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::batch_reduce_map_program_builder<op, kernel_factory> \
	)

/**
 * @brief Register a CPU batch reduce-map builder with an explicit
 * dispatcher and indexing.
 *
 * @param name Identifier of the registration object.
 * @param op The operation.
 * @param kernel_factory The kernel factory.
 * @param ... The type dispatcher, followed optionally by the reduction
 * indexing and then the map indexing.
 */
#define REXLIB_REGISTER_BATCH_REDUCE_MAP_PROGRAM_BUILDER_EX( \
	name, op, kernel_factory, ... \
) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::batch_reduce_map_program_builder< \
			op, kernel_factory, __VA_ARGS__ \
		> \
	)

#include "batch_reduce_map_program_builder.inl"
