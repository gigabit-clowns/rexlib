// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_type_descriptor.hpp"
#include "operand_type_pivot.hpp"
#include "operand_type_rule.hpp"

#include <rex/core/dispatch/operation_data_type_policy.hpp>

#include <rex/core/numerical/numerical_type.hpp>
#include <rex/core/platform/dynamic_shared_object.h>
#include <rex/core/span.hpp>

#include <array>
#include <cstddef>
#include <initializer_list>

namespace rex

{

/**
 * @brief Data type policy driven by a declarative typing rule.
 *
 * Interprets the descriptor tables a rule lowers to, rather than the rule
 * itself, so a catalog of hundreds of operations shares one implementation
 * instead of instantiating one per rule.
 *
 * `deduce` resolves the rule's pivots from the input types, checks every
 * input against its slot and evaluates the output slots. `accept` binds the
 * rule's free pivots from the user supplied outputs and then re-checks every
 * operand, which subsumes the base class' equality check while still
 * admitting the looser outputs a converting operation needs.
 *
 * @see operand_type_rule
 * @see rule_data_type_policy
 */
class REXLIB_API rule_operation_data_type_policy final
	: public operation_data_type_policy
{
public:
	/**
	 * @brief Construct a policy from the tables of a rule.
	 *
	 * @param pivots The rule's pivot table.
	 * @param output_slots The rule's output slot table.
	 * @param input_slots The rule's input slot table.
	 */
	rule_operation_data_type_policy(
		span<const pivot_descriptor> pivots,
		span<const slot_descriptor> output_slots,
		span<const slot_descriptor> input_slots
	) noexcept;

	/**
	 * @brief Construct a policy for a rule with operation supplied pivots.
	 *
	 * Such a policy is owned by the operation instance rather than shared,
	 * because the values it carries are parameters of that instance.
	 *
	 * @param pivots The rule's pivot table.
	 * @param output_slots The rule's output slot table.
	 * @param input_slots The rule's input slot table.
	 * @param operation_pivots The values supplied by the operation. Must
	 * hold at most max_operand_type_pivot_count entries.
	 */
	rule_operation_data_type_policy(
		span<const pivot_descriptor> pivots,
		span<const slot_descriptor> output_slots,
		span<const slot_descriptor> input_slots,
		std::initializer_list<numerical_type> operation_pivots
	) noexcept;

	~rule_operation_data_type_policy() override;

	void deduce(
		const operation_descriptor &descriptor,
		span<numerical_type> canonical_output_types,
		span<const numerical_type> input_types
	) const override;

	void accept(
		const operation_descriptor &descriptor,
		span<const numerical_type> user_output_types,
		span<const numerical_type> canonical_output_types,
		span<const numerical_type> input_types
	) const override;

private:
	REXLIB_STD_MEMBER_INTERFACE
	span<const pivot_descriptor> m_pivots;
	REXLIB_STD_MEMBER_INTERFACE
	span<const slot_descriptor> m_output_slots;
	REXLIB_STD_MEMBER_INTERFACE
	span<const slot_descriptor> m_input_slots;

	REXLIB_STD_MEMBER_INTERFACE
	std::array<numerical_type, max_operand_type_pivot_count>
		m_operation_pivots;
	std::size_t m_operation_pivot_count;

	span<const numerical_type> get_operation_pivots() const noexcept;
	span<const slot_descriptor> get_slots_for_report(
		bool output_operands
	) const noexcept;
};

/**
 * @brief Shared data type policy of a typing rule.
 *
 * Names the policy of a rule as a tag type, so that an operation can refer
 * to it in its declaration. The policy is stateless and therefore shared by
 * every instance of every operation using that rule.
 *
 * @tparam Rule The operand_type_rule to interpret.
 */
template <typename Rule>
struct rule_data_type_policy
{
	static_assert(
		!detail::rule_has_operation_pivot<
			typename Rule::pivot_list
		>::value,
		"A rule with an operation supplied pivot cannot use a shared "
		"policy, because its pivot values are parameters of the operation "
		"instance. Hold a rule_operation_data_type_policy as a member "
		"instead."
	);

	/**
	 * @brief Get the policy interpreting the rule.
	 *
	 * @return const operation_data_type_policy& The policy.
	 */
	static const operation_data_type_policy& get() noexcept;
};

} // namespace rex

#include "rule_operation_data_type_policy.inl"
