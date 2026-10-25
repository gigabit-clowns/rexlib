// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/meta/type_list.hpp>

#include <type_traits>

namespace rexlib
{

/**
 * @brief Check that each pivot type lies in the domain its pivot declares.
 *
 * @tparam Pivots type_list of type_pivot declarations.
 * @tparam Types type_list of the candidate pivot element types.
 */
template <typename Pivots, typename Types>
struct type_rule_pivots_in_domain;

template <>
struct type_rule_pivots_in_domain<type_list<>, type_list<>>
	: std::true_type
{
};

template <
	typename Pivot, typename... Pivots,
	typename Type, typename... Types
>
struct type_rule_pivots_in_domain<
	type_list<Pivot, Pivots...>,
	type_list<Type, Types...>
>
	: std::integral_constant<
		bool,
		Pivot::domain_type::template contains_type<Type>::value &&
		type_rule_pivots_in_domain<
			type_list<Pivots...>,
			type_list<Types...>
		>::value
	>
{
};

} // namespace rexlib
