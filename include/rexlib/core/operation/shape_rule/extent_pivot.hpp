// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/any_extent_domain.hpp>
#include <rexlib/core/operation/shape_rule/extent_pivot_source.hpp>
#include <rexlib/core/operation/shape_rule/fixed_extent.hpp>

#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Maximum number of extent variables a shape rule may declare.
 *
 * Bounded so that a resolved rule stays an allocation free value on the
 * dispatch path. Realistic rules use up to three.
 */
REXLIB_INLINE_CONST_CONSTEXPR
std::size_t max_extent_pivot_count = 8;

/**
 * @brief Declaration of one extent variable of a shape rule.
 *
 * An extent variable is the extent several core axes share or derive from,
 * such as the inner extent of a matrix multiplication. Core axes never
 * broadcast, so every axis naming the variable must agree with it.
 *
 * @tparam Source Where the value comes from.
 * @tparam Accessor Type reading the value from the parameters of the
 * operation instance, through a static `get(const Parameters&)` template.
 * void when @p Source is extent_pivot_source::inputs.
 * @tparam Domain Tag naming the extents the variable may take.
 *
 * @see operand_shape_rule
 */
template <extent_pivot_source Source, typename Accessor, typename Domain>
struct extent_pivot
{
	static REXLIB_CONST_CONSTEXPR extent_pivot_source source = Source;

	using accessor_type = Accessor;
	using domain_type = Domain;
};

/**
 * @brief Extent variable bound by the input axes that name it.
 *
 * @tparam Domain Tag naming the admissible extents.
 */
template <typename Domain = any_extent_domain>
using extent_from_inputs =
	extent_pivot<extent_pivot_source::inputs, void, Domain>;

/**
 * @brief Extent variable supplied by the operation instance.
 *
 * Models operations that fix an extent by themselves, such as the length of
 * a sequence.
 *
 * @tparam Accessor Type reading the extent from the parameters of the
 * operation instance.
 * @tparam Domain Tag naming the admissible extents.
 */
template <typename Accessor, typename Domain = any_extent_domain>
using extent_from_operation =
	extent_pivot<extent_pivot_source::operation, Accessor, Domain>;

/**
 * @brief Extent variable fixed by the declaration of the operation.
 *
 * Models core axes of a fixed length, such as the three components of a
 * cross product. The value reaches the interpreter with the extents the
 * operation instance supplies, so a rule reading only fixed extents needs
 * no parameters.
 *
 * @tparam N The extent.
 */
template <std::size_t N>
using extent_fixed = extent_pivot<
	extent_pivot_source::fixed,
	fixed_extent<N>,
	any_extent_domain
>;

} // namespace rexlib
