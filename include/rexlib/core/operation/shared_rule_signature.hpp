// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/operand_type_rule.hpp>

#include <rexlib/core/operation/operation_signature.hpp>

namespace rexlib
{

/**
 * @brief The signature of every operation that takes no parameters and
 * follows a given pair of rules.
 *
 * Such a signature has nothing that belongs to one instance, so one is
 * shared by every instance of every operation following the same rules.
 *
 * @tparam ShapeRule The operand_shape_rule. It must read nothing from the
 * operation instance.
 * @tparam TypeRule The operand_type_rule. It must read nothing from the
 * operation instance.
 *
 * @see rule_signature
 */
template <typename ShapeRule, typename TypeRule>
struct shared_rule_signature
{
	static_assert(
		!ShapeRule::requires_parameters,
		"A shape rule reading values of the operation instance cannot be "
		"shared. Own a rule_signature holding the parameters "
		"instead."
	);

	static_assert(
		!detail::rule_has_operation_pivot<
			typename TypeRule::pivot_list
		>::value,
		"A typing rule with an operation supplied pivot cannot be shared. "
		"Own a rule_signature holding the parameters instead."
	);

	/**
	 * @brief Get the signature.
	 *
	 * @return const operation_signature& The signature.
	 */
	static const operation_signature& get() noexcept;
};

} // namespace rexlib

#include "shared_rule_signature.inl"
