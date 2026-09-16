// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/backends/cpu/program_builder.hpp>

#include <rexlib/core/dispatch/program_builder_registry.hpp>
#include <rexlib/core/platform/cpp_attributes.hpp>

#include <core/dispatch/core_program_builder_registry.hpp>

#include <backends/cpu/builders/dispatched_program_builder.hpp>
#include <backends/cpu/builders/program_builder_registration.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>
#include <backends/cpu/loops/element_index_tags.hpp>
#include <backends/cpu/plans/reduction_layout_plan.hpp>

#include <rexlib/core/meta/type_list.hpp>

#include <memory>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Generic program builder for the reduction operation family.
 *
 * Builds a CPU program that folds the elements of its inputs along the axes
 * the operation names. It owns the scaffolding shared by every reduction:
 * operation type check, operand-count validation, the split of the iteration
 * space into the axes that survive and the axes being folded away, and type
 * dispatch.
 *
 * The axes are a parameter of the operation rather than of the call, so the
 * whole of that split is decided once here and the program that results holds
 * no decision left to make per element.
 *
 * @tparam Op The operation type this builder targets. Must carry a
 * reduction_operation_shape_policy.
 * @tparam KernelFactory Factory producing the reduction kernel. Invoked as
 * factory(operation, output_types, input_types), where operation is the
 * concrete @p Op and the type arguments are type_list-s of the resolved
 * element types. The returned kernel describes the fold through the members
 * @ref run_indexed_reduction_loop documents. Use default_kernel_factory for
 * stateless kernels, and fold_reduction_kernel for an operation that is no
 * more than a binary fold with a neutral element.
 * @tparam TypeDispatcher The type dispatch policy. It resolves the runtime
 * operand data types into compile-time element types. It defaults to
 * interpreting the operation's own typing rule, so a builder only names one
 * when this backend supports a narrower set of element types than the
 * operation itself allows.
 * @tparam Indexing The index the kernel is handed within the reduced space:
 * nothing for @ref no_index_tag, the linear index of the element for
 * @ref linear_index_tag, and its coordinates for
 * @ref multidimensional_index_tag.
 */
template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatcher = rule_type_dispatcher<typename Op::type_rule>,
	typename Indexing = no_index_tag
>
class reduction_program_builder final
	: public dispatched_program_builder<
		reduction_program_builder<Op, KernelFactory, TypeDispatcher, Indexing>,
		Op,
		TypeDispatcher
	>
{
public:
	/**
	 * @brief Split the iteration space into the axes that survive and the
	 * axes being folded away.
	 *
	 * The axes are a parameter of the operation rather than of the call, so
	 * the whole of that split is decided here and the program that results
	 * holds no decision left to make per element.
	 *
	 * @param operation The operation.
	 * @param output_signatures The output operand signatures.
	 * @param input_signatures The input operand signatures.
	 * @return reduction_layout_plan The planned iteration.
	 */
	reduction_layout_plan make_plan(
		const Op &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures
	) const;

	/**
	 * @brief Build the functor driving the reduction loop.
	 *
	 * @tparam Outs Element types of the outputs.
	 * @tparam Ins Element types of the inputs.
	 * @param operation The operation.
	 * @param plan The planned iteration, moved from.
	 * @param output_element_types Element types of the outputs.
	 * @param input_element_types Element types of the inputs.
	 * @return The functor the program runs.
	 */
	template <typename... Outs, typename... Ins>
	auto make_loop_functor(
		const Op &operation,
		reduction_layout_plan &plan,
		type_list<Outs...> output_element_types,
		type_list<Ins...> input_element_types
	) const;

private:
	using kernel_factory_type = KernelFactory;

	REXLIB_NO_UNIQUE_ADDRESS kernel_factory_type m_kernel_factory;
};

/**
 * @brief A reduction builder handing its kernel the linear index of every
 * element within the reduced space.
 *
 * Names the index so that a registration does not have to respell the
 * default type dispatcher merely to reach past it.
 *
 * @tparam Op The operation type this builder targets.
 * @tparam KernelFactory Factory producing the reduction kernel.
 */
template <typename Op, typename KernelFactory>
using linear_indexed_reduction_program_builder =
	reduction_program_builder<
		Op,
		KernelFactory,
		rule_type_dispatcher<typename Op::type_rule>,
		linear_index_tag
	>;

/**
 * @brief A reduction builder handing its kernel the coordinates of every
 * element within the reduced space.
 *
 * @tparam Op The operation type this builder targets.
 * @tparam KernelFactory Factory producing the reduction kernel.
 */
template <typename Op, typename KernelFactory>
using multidimensional_indexed_reduction_program_builder =
	reduction_program_builder<
		Op,
		KernelFactory,
		rule_type_dispatcher<typename Op::type_rule>,
		multidimensional_index_tag
	>;

} // namespace cpu
} // namespace rexlib

/**
 * @brief Instantiate and auto-register a CPU reduction program builder.
 *
 * The operand element types are dispatched by interpreting the operation's
 * own typing rule, so the builder states nothing about typing.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the reduction kernel.
 */
#define REXLIB_REGISTER_REDUCTION_PROGRAM_BUILDER(name, op, kernel_factory) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::reduction_program_builder<op, kernel_factory> \
	)

/**
 * @brief Instantiate and auto-register a CPU reduction builder whose kernel
 * is handed the linear index of every element within the reduced space.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the reduction kernel.
 */
#define REXLIB_REGISTER_LINEAR_INDEXED_REDUCTION_PROGRAM_BUILDER( \
	name, op, kernel_factory \
) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::linear_indexed_reduction_program_builder< \
			op, kernel_factory \
		> \
	)

/**
 * @brief Instantiate and auto-register a CPU reduction builder whose kernel
 * is handed the coordinates of every element within the reduced space.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the reduction kernel.
 */
#define REXLIB_REGISTER_MULTIDIMENSIONAL_INDEXED_REDUCTION_PROGRAM_BUILDER( \
	name, op, kernel_factory \
) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::multidimensional_indexed_reduction_program_builder< \
			op, kernel_factory \
		> \
	)

/**
 * @brief Register a CPU reduction builder with an explicit dispatcher.
 *
 * Only needed when this backend supports a narrower set of element types
 * than the operation allows, or for an operation that does not declare a
 * typing rule yet.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the reduction kernel.
 * @param ... The type dispatcher. It comes last so that the commas in its
 * template arguments do not split the macro arguments.
 */
#define REXLIB_REGISTER_REDUCTION_PROGRAM_BUILDER_EX( \
	name, op, kernel_factory, ... \
) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::reduction_program_builder< \
			op, kernel_factory, __VA_ARGS__ \
		> \
	)

#include "reduction_program_builder.inl"
