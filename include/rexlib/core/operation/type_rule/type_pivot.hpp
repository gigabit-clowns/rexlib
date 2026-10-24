// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/type_pivot_source.hpp>

#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Maximum number of free variables a typing rule may declare.
 *
 * Bounded so that a resolved rule stays a trivially copyable, allocation
 * free value on the dispatch path. Realistic rules use one or two pivots.
 */
REXLIB_INLINE_CONST_CONSTEXPR
std::size_t max_type_pivot_count = 8;

/**
 * @brief Declaration of one free variable of a typing rule.
 *
 * A rule resolves its pivots first and then derives every operand type from
 * them, so the pivots are what a backend has to dispatch over. Declaring
 * where a pivot comes from, rather than inferring it, is what lets `accept`
 * tell a free pivot from a fixed one.
 *
 * @tparam Source Where the pivot value comes from.
 * @tparam Index Index of the operand supplying it. Unused when @p Source
 * is type_pivot_source::operation.
 * @tparam Domain Tag naming the set of types the pivot may take. It is
 * intersected with the domain of every transform reading this pivot, so it
 * only ever narrows.
 * @tparam DefaultPivot Only meaningful when @p Source is
 * type_pivot_source::output. When non negative, the index of the pivot whose
 * value this one adopts if the user did not pre-allocate the output. When
 * negative, the canonical type is left unknown for `accept` to resolve.
 * @tparam Accessor Only meaningful when @p Source is
 * type_pivot_source::operation. Type reading the value from the parameters of
 * the operation instance, through a static `get(const Parameters&)`
 * template. void otherwise.
 *
 * @see operand_type_rule
 */
template <
	type_pivot_source Source,
	std::size_t Index,
	typename Domain,
	std::ptrdiff_t DefaultPivot = -1,
	typename Accessor = void
>
struct type_pivot
{
	static REXLIB_CONST_CONSTEXPR type_pivot_source source = Source;
	static REXLIB_CONST_CONSTEXPR std::size_t index = Index;
	static REXLIB_CONST_CONSTEXPR std::ptrdiff_t default_pivot = DefaultPivot;

	using domain_type = Domain;
	using accessor_type = Accessor;
};

/**
 * @brief Pivot fixed by the data type of an input operand.
 *
 * @tparam I Index of the input operand.
 * @tparam Domain Tag naming the admissible types.
 */
template <std::size_t I, typename Domain = any_type_domain>
using pivot_from_input = type_pivot<type_pivot_source::input, I, Domain>;

/**
 * @brief Free pivot, adopted from a user supplied output operand.
 *
 * @tparam I Index of the output operand.
 * @tparam Domain Tag naming the admissible types.
 * @tparam DefaultPivot Index of the pivot whose value is adopted when the
 * user did not pre-allocate the output, or -1 to leave it unknown.
 */
template <
	std::size_t I,
	typename Domain = any_type_domain,
	std::ptrdiff_t DefaultPivot = -1
>
using pivot_from_output =
	type_pivot<type_pivot_source::output, I, Domain, DefaultPivot>;

/**
 * @brief Pivot supplied by the operation instance.
 *
 * Models operations that carry a type as a parameter, such as a cast to a
 * requested data type. The values the operation supplies are taken in the
 * order the pivots that read them are declared.
 *
 * @tparam Accessor Type reading the value from the parameters of the
 * operation instance.
 * @tparam Domain Tag naming the admissible types.
 */
template <typename Accessor, typename Domain = any_type_domain>
using pivot_from_operation =
	type_pivot<type_pivot_source::operation, 0, Domain, -1, Accessor>;

} // namespace rexlib
