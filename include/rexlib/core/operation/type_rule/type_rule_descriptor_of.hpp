// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/type_rule_descriptor.hpp>

namespace rexlib
{

/**
 * @brief Lower a type rule into a descriptor.
 *
 * @tparam Rule An operand_type_rule.
 */
template <typename Rule>
struct type_rule_descriptor_of
{
	/**
	 * @brief Get the descriptor of the rule.
	 *
	 * @return const type_rule_descriptor& The descriptor, in static
	 * storage.
	 */
	static const type_rule_descriptor& get() noexcept;
};

} // namespace rexlib

#include "type_rule_descriptor_of.inl"
