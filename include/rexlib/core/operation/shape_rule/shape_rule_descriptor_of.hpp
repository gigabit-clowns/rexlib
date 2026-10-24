// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/shape_rule_descriptor.hpp>

namespace rexlib
{

/**
 * @brief Lower a shape rule into its descriptor.
 *
 * @tparam Rule The operand_shape_rule to lower.
 */
template <typename Rule>
struct shape_rule_descriptor_of
{
	/**
	 * @brief Get the descriptor of the rule.
	 *
	 * @return const shape_rule_descriptor& The descriptor, shared by every
	 * caller.
	 */
	static const shape_rule_descriptor& get() noexcept;
};

} // namespace rexlib

#include "shape_rule_descriptor_of.inl"
