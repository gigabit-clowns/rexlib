// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/backends/cpu/program_builder.hpp>

#include <xmipp4/core/dispatch/program_builder_registry.hpp>
#include <xmipp4/core/platform/cpp_attributes.hpp>

#include <core/dispatch/core_program_builder_registry.hpp>

#include <backends/cpu/builders/dispatched_program_builder.hpp>
#include <backends/cpu/builders/program_builder_registration.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>
#include <backends/cpu/plans/reduction_layout_plan.hpp>

#include <xmipp4/core/meta/type_list.hpp>

#include <memory>

namespace rex
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
 * @ref run_reduction_loop documents. Use default_kernel_factory for stateless
 * kernels, and fold_reduction_kernel for an operation that is no more than a
 * binary fold with a neutral element.
 * @tparam TypeDispatcher The type dispatch policy. It resolves the runtime
 * operand data types into compile-time element types. It defaults to
 * interpreting the operation's own typing rule, so a builder only names one
 * when this backend supports a narrower set of element types than the
 * operation itself allows.
 * @tparam Ordered Whether the reduced space must be traversed in the order an
 * index into it counts, which an operation reporting where it found something
 * needs and which costs it the reordering for locality. See
 * @ref reduction_layout_plan.
 */
template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatcher = rule_type_dispatcher<typename Op::type_rule>,
	bool Ordered = false
>
class reduction_program_builder final
	: public dispatched_program_builder<
		reduction_program_builder<Op, KernelFactory, TypeDispatcher, Ordered>,
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

	REX_NO_UNIQUE_ADDRESS kernel_factory_type m_kernel_factory;
};

/**
 * @brief A reduction builder reporting where it found something.
 *
 * Names the traversal requirement so that a registration does not have to
 * respell the default type dispatcher merely to reach past it.
 *
 * @tparam Op The operation type this builder targets.
 * @tparam KernelFactory Factory producing the reduction kernel.
 */
template <typename Op, typename KernelFactory>
using indexed_reduction_program_builder =
	reduction_program_builder<
		Op,
		KernelFactory,
		rule_type_dispatcher<typename Op::type_rule>,
		true
	>;

} // namespace cpu
} // namespace rex

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
#define REX_REGISTER_REDUCTION_PROGRAM_BUILDER(name, op, kernel_factory) \
	REX_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::xmipp4::cpu::reduction_program_builder<op, kernel_factory> \
	)

/**
 * @brief Instantiate and auto-register a CPU reduction builder that reports
 * where in the reduced space it found something.
 *
 * Traverses the reduced space in the order an index into it counts, giving up
 * the reordering for locality so that the position the kernel is handed means
 * the same thing whatever the operand's layout.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the reduction kernel.
 */
#define REX_REGISTER_INDEXED_REDUCTION_PROGRAM_BUILDER( \
	name, op, kernel_factory \
) \
	REX_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::xmipp4::cpu::indexed_reduction_program_builder<op, kernel_factory> \
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
#define REX_REGISTER_REDUCTION_PROGRAM_BUILDER_EX( \
	name, op, kernel_factory, ... \
) \
	REX_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::xmipp4::cpu::reduction_program_builder< \
			op, kernel_factory, __VA_ARGS__ \
		> \
	)

#include "reduction_program_builder.inl"
