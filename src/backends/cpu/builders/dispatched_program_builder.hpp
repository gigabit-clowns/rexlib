// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/backends/cpu/program_builder.hpp>

#include <rex/core/dispatch/operand_signature.hpp>
#include <rex/core/numerical/numerical_type.hpp>

#include <backends/cpu/builders/operand_data_types.hpp>

#include <array>
#include <cstddef>
#include <memory>

namespace rex
{
namespace cpu
{

/**
 * @brief Base for a program builder that resolves its operand element types.
 *
 * Every CPU builder family answers the same three questions the same way,
 * and differs only in what it plans and what it runs. This owns the part
 * they share: the operation identity, the whole of @ref get_suitability, the
 * operand count validation, the extraction of the operand element types and
 * the type dispatch that turns them into compile time ones. What is left to
 * a family is a plan and a functor.
 *
 * A derived builder supplies, by hiding the member of the same name where
 * one is given a default:
 *
 * @code
 * // The description of the iteration, settled once per program. Any type;
 * // it is handed straight back to make_loop_functor.
 * Plan make_plan(const Op &, output_signatures, input_signatures) const;
 *
 * // The functor the program runs, for one resolved set of element types.
 * // The plan is passed by mutable reference so it can be moved from.
 * F make_loop_functor(const Op &, Plan &, type_list<Outs...>,
 *                     type_list<Ins...>) const;
 *
 * // Admissibility beyond operand counts and element types, such as a rank
 * // an operand must reach. Accepts by default.
 * bool accepts_signatures(output_signatures, input_signatures) const noexcept;
 * @endcode
 *
 * These are the contract between a family and this base rather than part of
 * the @ref program_builder interface, and are public only because the base
 * reaches them through @p Derived.
 *
 * @tparam Derived The builder deriving from this, for static dispatch to the
 * members above.
 * @tparam Op The operation type the derived builder targets.
 * @tparam TypeDispatcher The type dispatch policy. It resolves the runtime
 * operand data types into compile-time element types, and is used through its
 * static members, so it is a policy rather than a stored object.
 */
template <typename Derived, typename Op, typename TypeDispatcher>
class dispatched_program_builder
	: public program_builder
{
public:
	dispatched_program_builder() noexcept = default;
	~dispatched_program_builder() override = default;

	operation_id get_operation_id() const noexcept override;

	backend_priority get_suitability(
		const operation &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		xmipp4::command_queue &queue
	) const override;

	std::shared_ptr<xmipp4::program> build(
		const operation &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		xmipp4::command_queue &queue,
		program_cache *cache
	) const override;

	/**
	 * @brief Whether the operands are admissible beyond their counts and
	 * element types.
	 *
	 * Accepts, which is the answer for a family whose whole requirement the
	 * operand counts and the typing rule already state. A family with more to
	 * ask, such as a rank an operand must reach, hides this.
	 *
	 * @param output_signatures The output operand signatures.
	 * @param input_signatures The input operand signatures.
	 * @return bool True.
	 */
	bool accepts_signatures(
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures
	) const noexcept;

private:
	/**
	 * @brief This builder as the type deriving from it.
	 *
	 * @return const Derived& This.
	 */
	const Derived &get_derived() const noexcept;
};

} // namespace cpu
} // namespace rex

#include "dispatched_program_builder.inl"
