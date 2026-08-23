// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/backends/cpu/program_builder.hpp>

#include <xmipp4/core/dispatch/program_builder_registry.hpp>
#include <xmipp4/core/platform/cpp_attributes.hpp>

#include <core/dispatch/core_program_builder_registry.hpp>

#include <backends/cpu/builders/dispatched_program_builder.hpp>
#include <backends/cpu/builders/program_builder_registration.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>
#include <backends/cpu/kernels/pocketfft_fourier_transform.hpp>
#include <backends/cpu/plans/fourier_layout_plan.hpp>

#include <xmipp4/core/meta/type_list.hpp>

#include <memory>

namespace rex
{
namespace cpu
{

/**
 * @brief Generic program builder for the Fourier transform family.
 *
 * Builds a CPU program that transforms one operand into another along the axes
 * the operation names, leaving every other axis as a batch dimension. It owns
 * the scaffolding the four transforms share: operation type check, operand
 * count validation, the reading of both operands' layouts into the shape,
 * strides and axes the transform is described by, and type dispatch. The
 * transform itself is delegated to pocketfft.
 *
 * The axes travel with the operation rather than with the call, so the whole
 * description of the iteration is settled once here and the program that
 * results holds nothing left to decide.
 *
 * @tparam Op The operation this builder targets. It must take one operand and
 * produce one, and carry a shape policy naming the transformed axes through
 * get_axes().
 * @tparam Transform The transform to perform. It is built from the scaling
 * convention the operation carries, names which domain each operand lives in
 * through a static get_kind(), and performs the transform when invoked as
 * transform(plan, output, input) with the operand pointers already displaced
 * by their offsets. See @ref complex_fourier_transform,
 * @ref real_to_complex_fourier_transform and
 * @ref complex_to_real_fourier_transform.
 * @tparam TypeDispatcher The type dispatch policy. It resolves the runtime
 * operand data types into compile time element types. It defaults to
 * interpreting the operation's own typing rule, narrowed to the precisions
 * pocketfft computes in.
 */
template <
	typename Op,
	typename Transform,
	typename TypeDispatcher = rule_type_dispatcher<
		typename Op::type_rule,
		fourier_transform_support
	>
>
class fourier_program_builder final
	: public dispatched_program_builder<
		fourier_program_builder<Op, Transform, TypeDispatcher>,
		Op,
		TypeDispatcher
	>
{
public:
	static_assert(
		Op::output_operand_count == 1,
		"A Fourier transform produces exactly one operand."
	);
	static_assert(
		Op::input_operand_count == 1,
		"A Fourier transform takes exactly one operand."
	);

	fourier_program_builder() noexcept = default;

	/**
	 * @brief Describe the transform over the operands.
	 *
	 * The axes travel with the operation, so the whole description is
	 * settled here, once per program rather than once per run.
	 *
	 * @param operation The operation.
	 * @param output_signatures The output operand signatures.
	 * @param input_signatures The input operand signatures.
	 * @return fourier_layout_plan The planned transform.
	 */
	fourier_layout_plan make_plan(
		const Op &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures
	) const;

	/**
	 * @brief Build the functor running the transform.
	 *
	 * There is no kernel factory: the transform is the functor's payload,
	 * and is built from the operation's own normalization.
	 *
	 * @tparam Outs Element types of the outputs.
	 * @tparam Ins Element types of the inputs.
	 * @param operation The operation.
	 * @param plan The planned transform, moved from.
	 * @param output_element_types Element types of the outputs.
	 * @param input_element_types Element types of the inputs.
	 * @return The functor the program runs.
	 */
	template <typename... Outs, typename... Ins>
	auto make_loop_functor(
		const Op &operation,
		fourier_layout_plan &plan,
		type_list<Outs...> output_element_types,
		type_list<Ins...> input_element_types
	) const;

};

} // namespace cpu
} // namespace rex

/**
 * @brief Instantiate and auto-register a CPU Fourier program builder.
 *
 * The operand element types are dispatched by interpreting the operation's own
 * typing rule, narrowed to the precisions pocketfft computes in, so the
 * builder states nothing about typing.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param transform The transform to perform.
 */
#define REX_REGISTER_FOURIER_PROGRAM_BUILDER(name, op, transform) \
	REX_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::xmipp4::cpu::fourier_program_builder<op, transform> \
	)

/**
 * @brief Register a CPU Fourier program builder with an explicit dispatcher.
 *
 * Only needed when this backend supports a narrower set of element types than
 * the default narrowing already expresses.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param transform The transform to perform.
 * @param ... The type dispatcher. It comes last so that the commas in its
 * template arguments do not split the macro arguments.
 */
#define REX_REGISTER_FOURIER_PROGRAM_BUILDER_EX( \
	name, op, transform, ... \
) \
	REX_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::xmipp4::cpu::fourier_program_builder< \
			op, transform, __VA_ARGS__ \
		> \
	)

#include "fourier_program_builder.inl"
