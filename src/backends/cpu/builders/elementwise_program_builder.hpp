// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/backends/cpu/program_builder.hpp>

#include <rexlib/core/dispatch/program_builder_registry.hpp>
#include <rexlib/core/platform/cpp_attributes.hpp>

#include <core/dispatch/core_program_builder_registry.hpp>

#include <backends/cpu/builders/dispatched_program_builder.hpp>
#include <backends/cpu/builders/program_builder_registration.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>

#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/meta/type_list.hpp>

#include <memory>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Generic program builder for the elementwise operation family.
 *
 * Builds a CPU program that applies a per-element kernel to every element of
 * the operands, following broadcast semantics. It owns the scaffolding shared
 * by every elementwise operation: operation type check, operand-count
 * validation, joint layout construction and type dispatch.
 *
 * @tparam Op The operation type this builder targets.
 * @tparam KernelFactory Factory producing the per-element kernel. Invoked as
 * factory(operation, output_types, input_types), where operation is the
 * concrete @p Op and the type arguments are type_list-s of the resolved
 * element types. The returned kernel is invoked with one mutable pointer per
 * output operand, in signature order, followed by one const pointer per input
 * operand, in signature order. Use default_kernel_factory for stateless
 * kernels.
 * @tparam TypeDispatcher The type dispatch policy. It resolves the runtime
 * operand data types into compile-time element types. It defaults to
 * interpreting the operation's own typing rule, so a builder only names one
 * when this backend supports a narrower set of element types than the
 * operation itself allows.
 */
template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatcher = rule_type_dispatcher<typename Op::type_rule>
>
class elementwise_program_builder final
	: public dispatched_program_builder<
		elementwise_program_builder<Op, KernelFactory, TypeDispatcher>,
		Op,
		TypeDispatcher
	>
{
public:
	elementwise_program_builder() noexcept = default;

	/**
	 * @brief Join the operand layouts into the one the loop walks.
	 *
	 * @param operation The operation.
	 * @param output_signatures The output operand signatures.
	 * @param input_signatures The input operand signatures.
	 * @return joint_layout The layout every operand is walked through.
	 */
	joint_layout make_plan(
		const Op &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures
	) const;

	/**
	 * @brief Build the functor driving the elementwise loop.
	 *
	 * @tparam Outs Element types of the outputs.
	 * @tparam Ins Element types of the inputs.
	 * @param operation The operation.
	 * @param layout The joined layout, moved from.
	 * @param output_element_types Element types of the outputs.
	 * @param input_element_types Element types of the inputs.
	 * @return The functor the program runs.
	 */
	template <typename... Outs, typename... Ins>
	auto make_loop_functor(
		const Op &operation,
		joint_layout &layout,
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
 * @brief Register a CPU elementwise builder with an explicit dispatcher.
 *
 * Only needed when this backend supports a narrower set of element types
 * than the operation allows, or for an operation that does not declare a
 * typing rule yet.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the per-element kernel.
 * @param ... The type dispatcher. It comes last so that the commas in its
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
