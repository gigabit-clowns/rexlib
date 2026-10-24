// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/any_extent_domain.hpp>
#include <rexlib/core/operation/shape_rule/axis_run.hpp>
#include <rexlib/core/operation/shape_rule/extent_slot.hpp>

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>
#include <type_traits>

namespace rexlib
{

namespace detail
{

template <typename T>
struct is_axis_run
	: std::false_type
{
};

template <typename Domain>
struct is_axis_run<axis_run<Domain>>
	: std::true_type
{
};

template <typename... Slots>
struct core_run_split
{
	static REXLIB_CONST_CONSTEXPR bool has_run = false;

	using run_domain_type = any_extent_domain;
	using slot_list = type_list<Slots...>;
};

template <typename Domain, typename... Slots>
struct core_run_split<axis_run<Domain>, Slots...>
{
	static REXLIB_CONST_CONSTEXPR bool has_run = true;

	using run_domain_type = Domain;
	using slot_list = type_list<Slots...>;
};

template <typename Slots>
struct slots_without_run;

template <typename... Slots>
struct slots_without_run<type_list<Slots...>>
	: std::is_same<
		type_list<std::false_type, typename is_axis_run<Slots>::type...>,
		type_list<typename is_axis_run<Slots>::type..., std::false_type>
	>
{
};

} // namespace detail

/**
 * @brief The core axes of one operand.
 *
 * Every axis of an operand that is not core is a batch axis, broadcast
 * across the operands.
 *
 * @tparam Slots An optional leading axis_run, followed by one extent_slot
 * per further core axis, in the order the axes are laid out.
 *
 * @see operand_shape_rule
 */
template <typename... Slots>
struct core_dimensions
{
	using slot_list = typename detail::core_run_split<Slots...>::slot_list;
	using run_domain_type =
		typename detail::core_run_split<Slots...>::run_domain_type;

	static REXLIB_CONST_CONSTEXPR bool has_run =
		detail::core_run_split<Slots...>::has_run;
	static REXLIB_CONST_CONSTEXPR std::size_t axis_count =
		type_list_size<slot_list>::value;

	static_assert(
		detail::slots_without_run<slot_list>::value,
		"A run may only lead the core dimensions, so the axes of one cannot "
		"be told from those of the other."
	);
};

} // namespace rexlib
