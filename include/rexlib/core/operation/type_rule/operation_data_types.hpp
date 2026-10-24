// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/type_rule_accessors.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>

#include <array>

namespace rexlib
{

/**
 * @brief Get the data types an operation instance supplies to its typing
 * rule.
 *
 * Evaluates the accessors of the rule's operation supplied pivots on the
 * parameters of the operation. An operation whose rule has none supplies
 * none, whether it has parameters or not.
 *
 * @tparam Op The concrete operation type.
 * @param operation The operation instance.
 * @return The data types, in the order their pivots are declared.
 */
template <typename Op>
std::array<
	numerical_type,
	type_rule_accessors<typename Op::type_rule>::operation_pivot_count
>
get_operation_data_types(const Op &operation) noexcept;

} // namespace rexlib

#include "operation_data_types.inl"
