// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/backends/cpu/program_builder.hpp>

#include <rex/core/dispatch/program_builder_registry.hpp>
#include <rex/core/platform/cpp_attributes.hpp>

#include <core/dispatch/core_program_builder_registry.hpp>

#include <backends/cpu/builders/dispatched_program_builder.hpp>
#include <backends/cpu/builders/program_builder_registration.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>
#include <backends/cpu/plans/sequence_layout_plan.hpp>

#include <rex/core/meta/type_list.hpp>

#include <memory>

namespace rex
{
namespace cpu
{

/**
 * @brief Generic program builder for the sequence creation family.
 *
 * Builds a CPU program that writes a one dimensional operand from nothing
 * but the position of each element. It owns the scaffolding shared by every
 * such operation: operation type check, operand-count validation, traversal
 * planning and type dispatch.
 *
 * The family is single output and takes no input. Both are checked
 * statically rather than left general: a sequence is what an operation
 * writes when it has nothing to read, and an operation writing two of them
 * at once would be two operations sharing a loop, which is a thing this
 * backend has no way to describe and no operation asking for.
 *
 * @tparam Op The operation type this builder targets.
 * @tparam KernelFactory Factory producing the generator. Invoked as
 * factory(operation, output_types, input_types), where operation is the
 * concrete @p Op and the type arguments are type_list-s of the resolved
 * element types. The returned generator is invoked once per element, with a
 * mutable pointer to the element and its position in the sequence.
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
class sequence_program_builder final
	: public dispatched_program_builder<
		sequence_program_builder<Op, KernelFactory, TypeDispatcher>,
		Op,
		TypeDispatcher
	>
{
public:
	sequence_program_builder() noexcept = default;

	/**
	 * @brief Whether the operand is the one dimensional one this writes.
	 *
	 * A rank other than one is not something this backend could do slower;
	 * it is something the shape policy already rules out. Reporting it as
	 * unsupported rather than throwing keeps that distinction, and lets the
	 * manager fall through if another backend disagrees.
	 *
	 * @param output_signatures The output operand signatures.
	 * @param input_signatures The input operand signatures.
	 * @return bool True if the output operand has rank one.
	 */
	bool accepts_signatures(
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures
	) const noexcept;

	/**
	 * @brief Settle the traversal of the operand.
	 *
	 * The whole traversal is three numbers off the layout, so it is settled
	 * here, once per program rather than once per element.
	 *
	 * @param operation The operation.
	 * @param output_signatures The output operand signatures.
	 * @param input_signatures The input operand signatures.
	 * @return sequence_layout_plan The planned traversal.
	 */
	sequence_layout_plan make_plan(
		const Op &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures
	) const;

	/**
	 * @brief Build the functor driving the sequence loop.
	 *
	 * @tparam Outs Element types of the outputs.
	 * @tparam Ins Element types of the inputs, empty.
	 * @param operation The operation.
	 * @param plan The planned traversal.
	 * @param output_element_types Element types of the outputs.
	 * @param input_element_types Element types of the inputs.
	 * @return The functor the program runs.
	 */
	template <typename... Outs, typename... Ins>
	auto make_loop_functor(
		const Op &operation,
		sequence_layout_plan &plan,
		type_list<Outs...> output_element_types,
		type_list<Ins...> input_element_types
	) const;

private:
	using kernel_factory_type = KernelFactory;

	static_assert(
		Op::output_operand_count == 1,
		"A sequence operation writes exactly one operand."
	);
	static_assert(
		Op::input_operand_count == 0,
		"A sequence operation is written from its parameters alone and "
		"takes no input operand."
	);

	REX_NO_UNIQUE_ADDRESS kernel_factory_type m_kernel_factory;
};

} // namespace cpu
} // namespace rex

/**
 * @brief Instantiate and auto-register a CPU sequence program builder.
 *
 * The operand element type is dispatched by interpreting the operation's own
 * typing rule, so the builder states nothing about typing.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the generator.
 */
#define REX_REGISTER_SEQUENCE_PROGRAM_BUILDER(name, op, kernel_factory) \
	REX_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::xmipp4::cpu::sequence_program_builder<op, kernel_factory> \
	)

/**
 * @brief Register a CPU sequence builder with an explicit dispatcher.
 *
 * Only needed when this backend supports a narrower set of element types
 * than the operation allows, or for an operation that does not declare a
 * typing rule yet.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the generator.
 * @param ... The type dispatcher. It comes last so that the commas in its
 * template arguments do not split the macro arguments.
 */
#define REX_REGISTER_SEQUENCE_PROGRAM_BUILDER_EX( \
	name, op, kernel_factory, ... \
) \
	REX_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::xmipp4::cpu::sequence_program_builder< \
			op, kernel_factory, __VA_ARGS__ \
		> \
	)

#include "sequence_program_builder.inl"
