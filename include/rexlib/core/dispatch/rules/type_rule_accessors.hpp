// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_type_pivot.hpp"

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <array>
#include <cstddef>
#include <type_traits>

namespace rexlib
{

namespace detail
{

template <typename Pivots>
struct operation_pivot_list;

template <>
struct operation_pivot_list<type_list<>>
{
	using type = type_list<>;
};

template <typename Head, typename... Tail>
struct operation_pivot_list<type_list<Head, Tail...>>
{
	using tail_type = typename operation_pivot_list<type_list<Tail...>>::type;
	using type = typename std::conditional<
		Head::source == pivot_source::operation,
		typename type_list_prepend<Head, tail_type>::type,
		tail_type
	>::type;
};

} // namespace detail

/**
 * @brief Read the data types a typing rule depends on from the parameters
 * of an operation instance.
 *
 * Evaluates the accessors the rule names, so that the data types can be
 * handed to a type_rule_interpreter, which knows nothing of parameter
 * types.
 *
 * @tparam Rule The operand_type_rule.
 */
template <typename Rule>
struct type_rule_accessors
{
	using operation_pivot_list = typename detail::operation_pivot_list<
		typename Rule::pivot_list
	>::type;

	static REXLIB_CONST_CONSTEXPR std::size_t operation_pivot_count =
		type_list_size<operation_pivot_list>::value;

	/**
	 * @brief Read every data type the operation supplies.
	 *
	 * @tparam Parameters The parameter type of the operation.
	 * @param parameters The parameters of the operation instance.
	 * @return std::array<numerical_type, operation_pivot_count> The data
	 * types, in the order their pivots are declared.
	 */
	template <typename Parameters>
	static std::array<numerical_type, operation_pivot_count>
	get_operation_pivots(const Parameters &parameters) noexcept;
};

} // namespace rexlib

#include "type_rule_accessors.inl"
