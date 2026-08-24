// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/meta/type_list.hpp>

#include <cstddef>
#include <type_traits>

namespace rexlib
{

/**
 * @brief Check that each pivot type lies in the domain its pivot declares.
 *
 * A backend uses this to decide whether to instantiate a program for a
 * candidate pivot combination at all. Without it, a kernel would have to
 * compile for element types the operation explicitly does not accept, which
 * is what the hand written backend predicates used to be for.
 *
 * @tparam Pivots type_list of operand_type_pivot declarations.
 * @tparam Types type_list of the candidate pivot element types.
 */
template <typename Pivots, typename Types>
struct rule_pivots_in_domain;

template <>
struct rule_pivots_in_domain<type_list<>, type_list<>>
	: std::true_type
{
};

template <
	typename Pivot, typename... Pivots,
	typename Type, typename... Types
>
struct rule_pivots_in_domain<
	type_list<Pivot, Pivots...>,
	type_list<Type, Types...>
>
	: std::integral_constant<
		bool,
		Pivot::domain_type::template contains_type<Type>::value &&
		rule_pivots_in_domain<
			type_list<Pivots...>,
			type_list<Types...>
		>::value
	>
{
};

/**
 * @brief Apply the transform of every slot to the type of its pivot.
 *
 * This is the type level counterpart of the runtime slot evaluation
 * performed by the rule engine. A backend resolves the pivots into
 * compile time element types and then uses this to reify the element type
 * of every operand, without knowing anything about the rule beyond its
 * slots.
 *
 * It is part of the public interface precisely so that out of tree backend
 * plugins can interpret a rule without re-deriving the metaprogramming.
 *
 * @tparam Slots type_list of operand_type_slot, one per operand.
 * @tparam Pivots type_list of the resolved pivot element types.
 *
 * @see operand_type_rule
 * @see operand_type_slot
 */
template <typename Slots, typename Pivots>
struct rule_operand_types;

template <typename... Slots, typename... Pivots>
struct rule_operand_types<type_list<Slots...>, type_list<Pivots...>>
{
	using type = type_list<
		typename Slots::transform_type::template apply_type<
			typename type_list_element<
				Slots::pivot_index,
				type_list<Pivots...>
			>::type
		>::type...
	>;
};

/**
 * @brief Convenience alias for `rule_operand_types<Slots, Pivots>::type`.
 *
 * @tparam Slots type_list of operand_type_slot, one per operand.
 * @tparam Pivots type_list of the resolved pivot element types.
 */
template <typename Slots, typename Pivots>
using rule_operand_types_t =
	typename rule_operand_types<Slots, Pivots>::type;

} // namespace rexlib
