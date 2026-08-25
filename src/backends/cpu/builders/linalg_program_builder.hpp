// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/backends/cpu/program_builder.hpp>

#include <rexlib/core/dispatch/program_builder_registry.hpp>
#include <rexlib/core/platform/cpp_attributes.hpp>

#include <core/dispatch/core_program_builder_registry.hpp>

#include <backends/cpu/builders/dispatched_program_builder.hpp>
#include <backends/cpu/builders/program_builder_registration.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>
#include <backends/cpu/plans/linalg_core_layout_plan.hpp>

#include <rexlib/core/meta/type_list.hpp>

#include <cstddef>
#include <memory>

namespace rexlib
{
namespace cpu
{

/**
 * @brief How many of an operation's trailing axes, per operand, are core.
 *
 * Specialized once per operation this builder serves. Two for a matrix
 * operand, one for a vector operand.
 */
template <typename Op>
struct linalg_core_ranks;

/**
 * @brief Generic program builder for the matmul/matvec/vecmat family.
 *
 * These operations all walk broadcastable batch axes plus a small, fixed
 * number of trailing "core" axes per operand (see linalg_core_layout_plan).
 * This builder owns the scaffolding shared by every one of them: operation
 * type check, operand-count validation, the batch/core split, and type
 * dispatch. cross is not one of them: its core axis is a caller-chosen
 * parameter rather than always the trailing one, so it implements
 * program_builder directly instead of going through this template.
 *
 * @tparam Op The operation type this builder targets. Must have a
 * linalg_core_ranks<Op> specialization.
 * @tparam KernelFactory Factory producing the per-batch-element kernel.
 * Invoked as factory(operation, output_types, input_types, plan), where the
 * type arguments are type_list-s of the resolved element types and plan is
 * the linalg_core_layout_plan built for this call. The returned kernel is
 * invoked once per batch element as kernel(out_ptr, left_ptr, right_ptr).
 * @tparam TypeDispatcher The type dispatch policy. Defaults to interpreting
 * the operation's own typing rule.
 */
template <
	typename Op,
	typename KernelFactory,
	typename TypeDispatcher = rule_type_dispatcher<typename Op::type_rule>
>
class linalg_program_builder final
	: public dispatched_program_builder<
		linalg_program_builder<Op, KernelFactory, TypeDispatcher>,
		Op,
		TypeDispatcher
	>
{
public:
	linalg_program_builder() noexcept = default;

	/**
	 * @brief Whether every operand reaches the rank its core needs.
	 *
	 * A rank smaller than the core it is asked to supply means the operand
	 * was promoted by the shape policy (matmul's vector promotion): this
	 * builder does not implement that, and declines rather than misreading
	 * a batch axis as a core one.
	 *
	 * @param output_signatures The output operand signatures.
	 * @param input_signatures The input operand signatures.
	 * @return bool True if every operand is at least as deep as its core.
	 */
	bool accepts_signatures(
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures
	) const noexcept;

	/**
	 * @brief Split every operand into its batch axes and its core.
	 *
	 * @param operation The operation.
	 * @param output_signatures The output operand signatures.
	 * @param input_signatures The input operand signatures.
	 * @return linalg_core_layout_plan The planned iteration.
	 */
	linalg_core_layout_plan make_plan(
		const Op &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures
	) const;

	/**
	 * @brief Build the functor driving the batch loop.
	 *
	 * The kernel factory is handed the plan as well as the element types,
	 * the core extents being what decides which kernel to produce.
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
		linalg_core_layout_plan &plan,
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
 * @brief Instantiate and auto-register a CPU matmul/matvec/vecmat family
 * program builder.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type. Must have a linalg_core_ranks<op>
 * specialization.
 * @param kernel_factory Factory producing the per-batch-element kernel.
 */
#define REXLIB_REGISTER_LINALG_PROGRAM_BUILDER(name, op, kernel_factory) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::linalg_program_builder<op, kernel_factory> \
	)

/**
 * @brief Register a CPU linalg builder with an explicit dispatcher.
 *
 * Only needed when this backend supports a narrower set of element types
 * than the operation allows.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param kernel_factory Factory producing the per-batch-element kernel.
 * @param ... The type dispatcher. It comes last so that the commas in its
 * template arguments do not split the macro arguments.
 */
#define REXLIB_REGISTER_LINALG_PROGRAM_BUILDER_EX( \
	name, op, kernel_factory, ... \
) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::linalg_program_builder< \
			op, kernel_factory, __VA_ARGS__ \
		> \
	)

#include "linalg_program_builder.inl"
