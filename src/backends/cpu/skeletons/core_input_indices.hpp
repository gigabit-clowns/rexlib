// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/meta/type_list.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace rexlib
{
namespace cpu
{

namespace detail
{

template <typename Cores, typename Selected, std::size_t Index>
struct select_core_inputs;

template <std::size_t... Selected, std::size_t Index>
struct select_core_inputs<
	type_list<>,
	std::index_sequence<Selected...>,
	Index
>
{
	using type = std::index_sequence<Selected...>;
};

template <
	typename First,
	typename... Rest,
	std::size_t... Selected,
	std::size_t Index
>
struct select_core_inputs<
	type_list<First, Rest...>,
	std::index_sequence<Selected...>,
	Index
>
	: select_core_inputs<
		type_list<Rest...>,
		typename std::conditional<
			(First::axis_count > 0 || First::has_run),
			std::index_sequence<Selected..., Index>,
			std::index_sequence<Selected...>
		>::type,
		Index + 1
	>
{
};

} // namespace detail

/**
 * @brief The indices of the inputs that carry core axes under a shape
 * rule.
 *
 * @tparam ShapeRule The operand_shape_rule.
 *
 * The member `type` is a `std::index_sequence` of the inputs whose core
 * dimensions are not empty, ascending.
 */
template <typename ShapeRule>
struct core_input_indices
	: detail::select_core_inputs<
		typename ShapeRule::input_core_list_type,
		std::index_sequence<>,
		0
	>
{
};

} // namespace cpu
} // namespace rexlib
