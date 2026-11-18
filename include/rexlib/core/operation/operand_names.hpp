// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/span.hpp>

#include <array>
#include <cstddef>
#include <type_traits>

namespace rexlib
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
class operand_names
{
public:
	static REXLIB_CONST_CONSTEXPR std::size_t count = N;

	/**
	 * @brief Construct the list from its names.
	 *
	 * @param names The operand names, in signature order.
	 */
	REXLIB_CONSTEXPR explicit operand_names(
		std::array<const char*, N> names
	) noexcept;

	/**
	 * @brief Get the names as a span.
	 *
	 * @return span<const char* const> The names.
	 */
	span<const char* const> get() const noexcept;

private:
	std::array<const char*, N> m_names;
};

/**
 * @brief Build an operand name list.
 *
 * @param names The operand names, in signature order.
 * @return operand_names<sizeof...(Names)> The name list.
 */
template <typename... Names>
REXLIB_CONSTEXPR
operand_names<sizeof...(Names)> make_operand_names(Names... names) noexcept;

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
		std::decay<T>::type::count
	>
{
};

} // namespace rexlib

#include "operand_names.inl"
