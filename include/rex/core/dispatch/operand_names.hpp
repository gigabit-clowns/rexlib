// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/constexpr.hpp"
#include "../span.hpp"

#include <array>
#include <cstddef>
#include <type_traits>

namespace rex
{

/**
 * @brief Compile time list of operand names.
 *
 * The extent doubles as the operand count, so an operation's names and its
 * arity are one declaration rather than two that could disagree.
 *
 * The names are not owned. They must have static storage duration, which
 * is the case for the string literals the declaration macros produce.
 *
 * @tparam N The number of operands.
 */
template <std::size_t N>
struct operand_names
{
	static REXLIB_CONST_CONSTEXPR std::size_t count = N;

	std::array<const char*, N> values;

	/**
	 * @brief Get the names as a span.
	 *
	 * @return span<const char* const> The names.
	 */
	span<const char* const> get() const noexcept
	{
		return make_span(values.data(), N);
	}
};

/**
 * @brief Build an operand name list.
 *
 * @param names The operand names, in signature order.
 * @return operand_names<sizeof...(Names)> The name list.
 */
template <typename... Names>
REXLIB_CONSTEXPR
operand_names<sizeof...(Names)> make_operand_names(Names... names) noexcept
{
	return operand_names<sizeof...(Names)>{ { names... } };
}

/**
 * @brief Get the operand count of an operand name list.
 *
 * Accepts references, so that it can be applied to the return type of a
 * traits accessor.
 *
 * @tparam T An operand_names specialization, possibly a reference to one.
 */
template <typename T>
struct operand_names_count
	: std::integral_constant<
		std::size_t,
		std::remove_cv<
			typename std::remove_reference<T>::type
		>::type::count
	>
{
};

} // namespace rex
