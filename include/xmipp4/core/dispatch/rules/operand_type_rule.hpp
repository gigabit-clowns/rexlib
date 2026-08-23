// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_type_pivot.hpp"
#include "operand_type_slot.hpp"

#include <xmipp4/core/meta/type_list.hpp>
#include <xmipp4/core/platform/constexpr.hpp>

#include <cstddef>
#include <type_traits>

namespace rex
{

namespace detail
{

template <std::size_t PivotCount, typename Slots>
struct slots_reference_valid_pivots;

template <std::size_t PivotCount>
struct slots_reference_valid_pivots<PivotCount, type_list<>>
	: std::true_type
{
};

template <std::size_t PivotCount, typename Head, typename... Tail>
struct slots_reference_valid_pivots<PivotCount, type_list<Head, Tail...>>
	: std::integral_constant<
		bool,
		(Head::pivot_index < PivotCount) &&
		slots_reference_valid_pivots<
			PivotCount,
			type_list<Tail...>
		>::value
	>
{
};

template <typename PivotList>
struct rule_has_operation_pivot;

template <>
struct rule_has_operation_pivot<type_list<>>
	: std::false_type
{
};

template <typename Head, typename... Tail>
struct rule_has_operation_pivot<type_list<Head, Tail...>>
	: std::integral_constant<
		bool,
		Head::source == pivot_source::operation ||
		rule_has_operation_pivot<type_list<Tail...>>::value
	>
{
};

} // namespace detail

/**
 * @brief Declarative typing rule for the operands of an operation.
 *
 * A rule declares a small set of free variables, the pivots, and binds every
 * operand to one of them through a transform. It is the single source of
 * truth for an operation's typing: two interpreters read this very same
 * declaration, and neither restates it.
 *
 *   * rule_operation_data_type_policy evaluates it on runtime
 *     numerical_type values, to deduce the canonical output types and to
 *     accept or reject user supplied ones.
 *   * The backend rule type dispatchers evaluate it on native C++ types, to
 *     reify the operand element types of the program being built. Because
 *     they dispatch the pivots rather than the operands, the number of
 *     instantiations they produce is bounded by the pivot count rather than
 *     by the operand count.
 *
 * The operand counts are part of the rule, so an operation's arity and the
 * compile time operand counts a backend builder needs both come from here
 * and cannot disagree.
 *
 * @tparam Pivots type_list of operand_type_pivot declarations.
 * @tparam Outputs type_list of operand_type_slot, one per output operand,
 * in signature order.
 * @tparam Inputs type_list of operand_type_slot, one per input operand, in
 * signature order.
 *
 * @see operand_type_pivot
 * @see operand_type_slot
 * @see operand_type_transform
 */
template <typename Pivots, typename Outputs, typename Inputs>
struct operand_type_rule
{
	using pivot_list = Pivots;
	using output_slot_list = Outputs;
	using input_slot_list = Inputs;

	static REX_CONST_CONSTEXPR std::size_t pivot_count =
		type_list_size<Pivots>::value;
	static REX_CONST_CONSTEXPR std::size_t output_count =
		type_list_size<Outputs>::value;
	static REX_CONST_CONSTEXPR std::size_t input_count =
		type_list_size<Inputs>::value;

	static_assert(
		pivot_count > 0,
		"An operand type rule must declare at least one pivot. With no free "
		"variable there is nothing for the backend to dispatch on."
	);

	static_assert(
		pivot_count <= max_operand_type_pivot_count,
		"An operand type rule declares more pivots than a resolution can "
		"hold. Raise max_operand_type_pivot_count if this is genuinely "
		"needed."
	);

	static_assert(
		detail::slots_reference_valid_pivots<pivot_count, Outputs>::value,
		"An output slot refers to a pivot the rule does not declare."
	);

	static_assert(
		detail::slots_reference_valid_pivots<pivot_count, Inputs>::value,
		"An input slot refers to a pivot the rule does not declare."
	);
};

} // namespace rex
