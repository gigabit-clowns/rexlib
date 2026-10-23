// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_axis_slot.hpp"

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>
#include <type_traits>

namespace rexlib
{

namespace detail
{

template <typename... Slots>
struct run_slot_count;

template <>
struct run_slot_count<>
	: std::integral_constant<std::size_t, 0>
{
};

template <typename Head, typename... Tail>
struct run_slot_count<Head, Tail...>
	: std::integral_constant<
		std::size_t,
		(Head::kind == axis_slot_kind::run ? 1 : 0) +
		run_slot_count<Tail...>::value
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
 * @tparam Slots operand_axis_slot declarations, one per core axis, in the
 * order the axes are laid out.
 *
 * @see operand_shape_rule
 */
template <typename... Slots>
struct core_dimensions
{
	using slot_list = type_list<Slots...>;

	static REXLIB_CONST_CONSTEXPR std::size_t slot_count = sizeof...(Slots);
	static REXLIB_CONST_CONSTEXPR std::size_t run_count =
		detail::run_slot_count<Slots...>::value;
	static REXLIB_CONST_CONSTEXPR std::size_t axis_count =
		slot_count - run_count;

	static_assert(
		run_count <= 1,
		"The core dimensions declare more than one run, so the axes of one "
		"cannot be told from those of the other."
	);
};

} // namespace rexlib
