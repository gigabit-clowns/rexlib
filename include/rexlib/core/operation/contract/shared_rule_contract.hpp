// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/rules/operand_type_rule.hpp>

#include <rexlib/core/operation/contract/operand_contract.hpp>

namespace rexlib
{

/**
 * @brief The contract of every operation that takes no parameters and
 * follows a given pair of rules.
 *
 * Such a contract has nothing that belongs to one instance, so one is
 * shared by every instance of every operation following the same rules.
 *
 * @tparam ShapeRule The operand_shape_rule. It must read nothing from the
 * operation instance.
 * @tparam TypeRule The operand_type_rule. It must read nothing from the
 * operation instance.
 *
 * @see rule_contract
 */
template <typename ShapeRule, typename TypeRule>
struct shared_rule_contract
{
	static_assert(
		!ShapeRule::requires_parameters,
		"A shape rule reading values of the operation instance cannot be "
		"shared. Own a rule_contract holding the parameters "
		"instead."
	);

	static_assert(
		!detail::rule_has_operation_pivot<
			typename TypeRule::pivot_list
		>::value,
		"A typing rule with an operation supplied pivot cannot be shared. "
		"Own a rule_contract holding the parameters instead."
	);

	/**
	 * @brief Get the contract.
	 *
	 * @return const operand_contract& The contract.
	 */
	static const operand_contract& get() noexcept;
};

} // namespace rexlib

#include "shared_rule_contract.inl"
