// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/backends/cpu/cpu_program_builder.hpp>

#include <rexlib/core/dispatch/program_builder_registry.hpp>
#include <rexlib/core/platform/cpp_attributes.hpp>


#include <backends/cpu/skeletons/typed_program_builder.hpp>
#include <backends/cpu/dispatch/program_builder_registration_macros.hpp>
#include <backends/cpu/skeletons/own_core_input_indices.hpp>
#include <backends/cpu/skeletons/rule_type_dispatch.hpp>
#include <backends/cpu/skeletons/elementwise/batched_map_plan.hpp>
#include <backends/cpu/loops/indices/element_index_tags.hpp>

#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/meta/type_list.hpp>

#include <memory>
#include <type_traits>

namespace rexlib
{

class joint_layout_builder;

namespace cpu
{

/**
 * @brief Generic program builder for the elementwise operation family.
 *
 * Builds a CPU program that applies a per-element kernel to every element of
 * the outputs. The shape rule of the operation places every input in the
 * frame of the outputs: an input without core axes broadcasts against the
 * axes before the core axes of the outputs, and an input carrying core axes
 * broadcasts against every axis. The shape rule must place its core axes
 * last, and the core of the outputs must have a fixed number of axes.
 *
 * An input may also have a core of its own, one the outputs do not carry,
 * such as the parameters an image is generated from. It is read once per
 * batch element: the kernel is then made for each batch element from the
 * inputs placed there, and walks its core, as batched_map_plan lays out.
 *
 * The builder owns the scaffolding shared by every elementwise operation:
 * operation type check, operand-count validation, joint layout construction
 * and type dispatch.
 *
 * @tparam Op The operation type this builder targets.
 * @tparam KernelFactory Factory producing the per-element kernel. Invoked as
 * factory(operation, output_types, input_types), where operation is the
 * concrete @p Op and the type arguments are type_list-s of the resolved
 * element types. The returned kernel is invoked with one mutable pointer per
 * output operand, in signature order, followed by one const pointer per input
 * operand, in signature order, followed by the index @p Indexing asks for.
 * Use default_kernel_factory for stateless kernels. With inputs of their own
 * core, the factory is invoked as factory(operation, output_types,
 * input_types, plan), plan being the batched_map_plan, and returns a kernel
 * maker: `maker.make_map(inputs)`, or `maker.make_map(inputs, counter)` for
 * @ref linear_index_tag, makes the kernel of a batch element from the tuple
 * of every input placed there and from the batch part of its counter. That
 * kernel is invoked as above, with the inputs the map walks only, and with
 * the index within the core.
 * @tparam TypeDispatch The type dispatch policy. It resolves the runtime
 * operand data types into compile-time element types. It defaults to
 * interpreting the operation's own typing rule, so a builder only names one
 * when this backend supports a narrower set of element types than the
 * operation itself allows.
 * @tparam Indexing The index the kernel is handed after its operand pointers,
 * within the space of the outputs: nothing for @ref no_index_tag, the linear
 * index of the element as a `std::size_t` for @ref linear_index_tag, and its
 * @ref multidimensional_index for @ref multidimensional_index_tag.
 */
template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatch = rule_type_dispatch<typename Op::type_rule_type>,
	typename Indexing = no_index_tag
>
class elementwise_program_builder final
	: public typed_program_builder<
		elementwise_program_builder<
			Op,
			KernelFactory,
			TypeDispatch,
			Indexing
		>,
		Op,
		TypeDispatch
	>
{
public:
	elementwise_program_builder() noexcept = default;

	/**
	 * @brief Join the operand layouts into the one the loop walks.
	 *
	 * @param operation The operation.
	 * @param output_descriptors The output operand descriptors.
	 * @param input_descriptors The input operand descriptors.
	 * @return The joint_layout every operand is walked through, followed by
	 * the index operands of @p Indexing, or the batched_map_plan when an
	 * input has a core of its own.
	 */
	auto make_plan(
		const Op &operation,
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors
	) const;

	/**
	 * @brief Build the functor driving the elementwise loop.
	 *
	 * @tparam Outs Element types of the outputs.
	 * @tparam Ins Element types of the inputs.
	 * @param operation The operation.
	 * @param plan The plan, moved from.
	 * @param output_element_types Element types of the outputs.
	 * @param input_element_types Element types of the inputs.
	 * @return The functor the program runs.
	 */
	template <typename Plan, typename... Outs, typename... Ins>
	auto make_loop_functor(
		const Op &operation,
		Plan &plan,
		type_list<Outs...> output_element_types,
		type_list<Ins...> input_element_types
	) const;

private:
	using kernel_factory_type = KernelFactory;
	using own_core_inputs_type =
		typename own_core_input_indices<typename Op::shape_rule_type>::type;
	using batched_type =
		std::integral_constant<bool, (own_core_inputs_type::size() > 0)>;

	joint_layout make_plan(
		const Op &operation,
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors,
		std::false_type batched
	) const;

	batched_map_plan make_plan(
		const Op &operation,
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors,
		std::true_type batched
	) const;

	template <typename... Outs, typename... Ins>
	auto make_loop_functor(
		const Op &operation,
		joint_layout &layout,
		type_list<Outs...> output_element_types,
		type_list<Ins...> input_element_types,
		std::false_type batched
	) const;

	template <typename... Outs, typename... Ins>
	auto make_loop_functor(
		const Op &operation,
		batched_map_plan &plan,
		type_list<Outs...> output_element_types,
		type_list<Ins...> input_element_types,
		std::true_type batched
	) const;

	void append_index_operands(
		joint_layout_builder &layout_builder,
		span<const operand_descriptor> output_descriptors,
		no_index_tag indexing
	) const noexcept;

	template <typename IndexTag>
	void append_index_operands(
		joint_layout_builder &layout_builder,
		span<const operand_descriptor> output_descriptors,
		IndexTag indexing
	) const;

	REXLIB_NO_UNIQUE_ADDRESS kernel_factory_type m_kernel_factory;
};

/**
 * @brief An elementwise builder handing its kernel the linear index of every
 * element.
 *
 * @tparam Op The operation type this builder targets.
 * @tparam KernelFactory Factory producing the per-element kernel.
 */
template <typename Op, typename KernelFactory>
using linear_indexed_elementwise_program_builder =
	elementwise_program_builder<
		Op,
		KernelFactory,
		rule_type_dispatch<typename Op::type_rule_type>,
		linear_index_tag
	>;

/**
 * @brief An elementwise builder handing its kernel the coordinates of every
 * element.
 *
 * @tparam Op The operation type this builder targets.
 * @tparam KernelFactory Factory producing the per-element kernel.
 */
template <typename Op, typename KernelFactory>
using multidimensional_indexed_elementwise_program_builder =
	elementwise_program_builder<
		Op,
		KernelFactory,
		rule_type_dispatch<typename Op::type_rule_type>,
		multidimensional_index_tag
	>;

} // namespace cpu
} // namespace rexlib

/**
 * @brief Instantiate and auto-register a CPU elementwise program builder.
 *
 * The operand element types are dispatched by interpreting the operation's
 * own typing rule, so the builder states nothing about typing.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the per-element kernel.
 */
#define REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(name, op, kernel_factory) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::elementwise_program_builder<op, kernel_factory> \
	)

/**
 * @brief Instantiate and auto-register a CPU elementwise program builder
 * whose kernel is handed the linear index of every element.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the per-element kernel.
 */
#define REXLIB_REGISTER_LINEAR_INDEXED_ELEMENTWISE_PROGRAM_BUILDER( \
	name, op, kernel_factory \
) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::linear_indexed_elementwise_program_builder< \
			op, kernel_factory \
		> \
	)

/**
 * @brief Instantiate and auto-register a CPU elementwise program builder
 * whose kernel is handed the coordinates of every element.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the per-element kernel.
 */
#define REXLIB_REGISTER_MULTIDIMENSIONAL_INDEXED_ELEMENTWISE_PROGRAM_BUILDER( \
	name, op, kernel_factory \
) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::multidimensional_indexed_elementwise_program_builder< \
			op, kernel_factory \
		> \
	)

/**
 * @brief Register a CPU elementwise builder with an explicit type dispatch.
 *
 * Only needed when this backend supports a narrower set of element types
 * than the operation allows, or for an operation that does not declare a
 * typing rule yet.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the per-element kernel.
 * @param ... The type dispatch policy. It comes last so that the commas in its
 * template arguments do not split the macro arguments.
 */
#define REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER_EX( \
	name, op, kernel_factory, ... \
) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::elementwise_program_builder< \
			op, kernel_factory, __VA_ARGS__ \
		> \
	)

#include "elementwise_program_builder.inl"
