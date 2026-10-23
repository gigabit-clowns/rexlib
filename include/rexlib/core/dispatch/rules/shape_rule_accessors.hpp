// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "named_core.hpp"
#include "operand_extent.hpp"
#include "trailing_core.hpp"

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/span.hpp>

#include <array>
#include <cstddef>
#include <type_traits>

namespace rexlib
{

namespace detail
{

template <typename Extents>
struct operation_extent_list;

template <>
struct operation_extent_list<type_list<>>
{
	using type = type_list<>;
};

template <typename Head, typename... Tail>
struct operation_extent_list<type_list<Head, Tail...>>
{
	using tail_type = typename operation_extent_list<type_list<Tail...>>::type;
	using type = typename std::conditional<
		Head::source == extent_source::operation,
		typename type_list_prepend<Head, tail_type>::type,
		tail_type
	>::type;
};

} // namespace detail

/**
 * @brief Read the values a shape rule depends on from the parameters of an
 * operation instance.
 *
 * Evaluates the accessors the rule names, so that the values can be handed
 * to a shape_rule_interpreter, which knows nothing of parameter types.
 *
 * @tparam Rule The operand_shape_rule.
 */
template <typename Rule>
struct shape_rule_accessors
{
	using operation_extent_list = typename detail::operation_extent_list<
		typename Rule::extent_list
	>::type;

	static REXLIB_CONST_CONSTEXPR std::size_t operation_extent_count =
		type_list_size<operation_extent_list>::value;

	/**
	 * @brief Read the axes the core axes lie along.
	 *
	 * @tparam Parameters The parameter type of the operation.
	 * @param parameters The parameters of the operation instance.
	 * @return span<const std::size_t> The axes, or none when the rule places
	 * the core axes last.
	 */
	template <typename Parameters>
	static span<const std::size_t> get_named_axes(
		const Parameters &parameters
	) noexcept;

	/**
	 * @brief Read whether an output without core axes keeps the named ones.
	 *
	 * @tparam Parameters The parameter type of the operation.
	 * @param parameters The parameters of the operation instance.
	 * @return bool What the keep accessor reads, or false when the rule
	 * names none.
	 */
	template <typename Parameters>
	static bool get_keep_dimensions(const Parameters &parameters) noexcept;

	/**
	 * @brief Read every extent the operation supplies.
	 *
	 * @tparam Parameters The parameter type of the operation.
	 * @param parameters The parameters of the operation instance.
	 * @return std::array<std::size_t, operation_extent_count> The extents,
	 * in the order their variables are declared.
	 */
	template <typename Parameters>
	static std::array<std::size_t, operation_extent_count>
	get_operation_extents(const Parameters &parameters) noexcept;
};

} // namespace rexlib

#include "shape_rule_accessors.inl"
