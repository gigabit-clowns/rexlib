// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/backends/cpu/program_builder.hpp>

#include <rex/core/dispatch/program_builder_registry.hpp>

#include <core/dispatch/core_program_builder_registry.hpp>

#include <backends/cpu/builders/dispatched_program_builder.hpp>
#include <backends/cpu/builders/program_builder_registration.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>

#include <rex/core/layout/joint_layout.hpp>
#include <rex/core/meta/type_list.hpp>

#include <memory>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief CPU program builder for cyclic axis shifts (fftshift/ifftshift).
 *
 * Both operations rearrange elements along a set of axes without changing
 * shape or type, differing only in how far each axis is shifted, so they
 * share this builder; @p ShiftPolicy supplies that one difference.
 *
 * @tparam Op The operation type, declared through
 * REXLIB_DECLARE_PARAMETRIC_OPERATION with an axiswise_operation_shape_policy.
 * @tparam ShiftPolicy Provides
 * `static std::size_t shift_amount(std::size_t extent) noexcept`, the
 * distance an axis of that extent is cyclically shifted by.
 * @tparam TypeDispatcher The type dispatch policy. A shift moves elements
 * without converting them, so the operation's own typing rule says all there
 * is to say and this is only named to keep the family uniform.
 */
template <
	typename Op,
	typename ShiftPolicy,
	typename TypeDispatcher = rule_type_dispatcher<typename Op::type_rule>
>
class roll_program_builder final
	: public dispatched_program_builder<
		roll_program_builder<Op, ShiftPolicy, TypeDispatcher>,
		Op,
		TypeDispatcher
	>
{
public:
	roll_program_builder() noexcept = default;

	/**
	 * @brief Carve the shift into blocks that copy without wrapping.
	 *
	 * The wraparound lives entirely in how the blocks are carved out, so
	 * that what runs per element is a plain affine copy.
	 *
	 * @param operation The operation.
	 * @param output_signatures The output operand signatures.
	 * @param input_signatures The input operand signatures.
	 * @return std::vector<joint_layout> One layout per block.
	 */
	std::vector<joint_layout> make_plan(
		const Op &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures
	) const;

	/**
	 * @brief Build the functor copying the blocks.
	 *
	 * @tparam Outs Element types of the outputs.
	 * @tparam Ins Element types of the inputs.
	 * @param operation The operation.
	 * @param blocks The blocks to copy, moved from.
	 * @param output_element_types Element types of the outputs.
	 * @param input_element_types Element types of the inputs.
	 * @return The functor the program runs.
	 */
	template <typename... Outs, typename... Ins>
	auto make_loop_functor(
		const Op &operation,
		std::vector<joint_layout> &blocks,
		type_list<Outs...> output_element_types,
		type_list<Ins...> input_element_types
	) const;
};

} // namespace cpu
} // namespace rexlib

/**
 * @brief Instantiate and auto-register a CPU roll program builder.
 *
 * @param name Identifier of the registration object.
 * @param op The operation type.
 * @param shift_policy The shift policy type.
 */
#define REXLIB_REGISTER_ROLL_PROGRAM_BUILDER(name, op, shift_policy) \
	REXLIB_REGISTER_CPU_PROGRAM_BUILDER( \
		name, \
		::rexlib::cpu::roll_program_builder<op, shift_policy> \
	)

#include "roll_program_builder.inl"
