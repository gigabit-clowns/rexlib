// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/rules/operand_type_rule.hpp>
#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @defgroup operand_type_rules Common operand type rules
 *
 * The typing rules the operation catalog is built from. An operation names
 * one of these in its declaration and that is the whole of its typing: the
 * runtime policy and every backend type dispatcher read the same rule.
 *
 * Every rule is parameterised by the domain of its element type, which is
 * how an operation states what it accepts. A rule says nothing about
 * shapes, so the same rule serves an elementwise operation and a reduction.
 *
 * @{
 */

/**
 * @brief Unary operation whose operands share one element type.
 *
 * @tparam Domain The admissible element types.
 */
template <typename Domain = any_type_domain>
using unary_homogeneous_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_same_as<0>>,
	type_list<slot_same_as<0>>
>;

/**
 * @brief Binary operation whose operands share one element type.
 *
 * @tparam Domain The admissible element types.
 */
template <typename Domain = any_type_domain>
using binary_homogeneous_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_same_as<0>>,
	type_list<slot_same_as<0>, slot_same_as<0>>
>;

/**
 * @brief Ternary operation whose operands share one element type.
 *
 * @tparam Domain The admissible element types.
 */
template <typename Domain = any_type_domain>
using ternary_homogeneous_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_same_as<0>>,
	type_list<slot_same_as<0>, slot_same_as<0>, slot_same_as<0>>
>;

/**
 * @brief Unary operation producing the real counterpart of its input.
 *
 * Models operations that drop the imaginary part, such as `abs`, and leaves
 * non complex inputs untouched.
 *
 * @tparam Domain The admissible input element types.
 */
template <typename Domain = arithmetic_type_domain>
using unary_real_of_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_real_of<0>>,
	type_list<slot_same_as<0>>
>;

/**
 * @brief Unary operation producing the complex counterpart of its input.
 *
 * @tparam Domain The admissible input element types.
 */
template <typename Domain = inexact_type_domain>
using unary_complex_of_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_complex_of<0>>,
	type_list<slot_same_as<0>>
>;

/**
 * @brief Unary operation producing the inexact counterpart of its input.
 *
 * Models operations whose result has a fractional part the operand's own
 * type could not hold, such as a mean, and leaves floating point and
 * complex inputs untouched.
 *
 * @tparam Domain The admissible input element types.
 */
template <typename Domain = arithmetic_type_domain>
using unary_inexact_of_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_inexact_of<0>>,
	type_list<slot_same_as<0>>
>;

/**
 * @brief Unary operation whose output has a fixed element type.
 *
 * Models operations whose result does not carry the input's type at all,
 * such as a predicate or one producing indices.
 *
 * @tparam Type The element type of the output.
 * @tparam Domain The admissible input element types.
 */
template <numerical_type Type, typename Domain = any_type_domain>
using unary_fixed_output_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_fixed<Type>>,
	type_list<slot_same_as<0>>
>;

/**
 * @brief Unary operation producing a boolean output.
 *
 * @tparam Domain The admissible input element types.
 */
template <typename Domain = any_type_domain>
using unary_predicate_rule = unary_fixed_output_rule<
	numerical_type::boolean,
	Domain
>;

/**
 * @brief Binary comparison producing a boolean output.
 *
 * @tparam Domain The admissible input element types.
 */
template <typename Domain = any_type_domain>
using binary_predicate_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_fixed<numerical_type::boolean>>,
	type_list<slot_same_as<0>, slot_same_as<0>>
>;

/**
 * @brief Unary operation producing two outputs of the operand type.
 *
 * Models operations computing two related results from one operand in a
 * single pass, such as the sine and the cosine of an angle.
 *
 * @tparam Domain The admissible element types.
 */
template <typename Domain = inexact_type_domain>
using unary_homogeneous_pair_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_same_as<0>, slot_same_as<0>>,
	type_list<slot_same_as<0>>
>;

/**
 * @brief Binary operation producing two outputs of the operand type.
 *
 * Models operations computing two related results in one pass, such as the
 * quotient and the remainder of a division.
 *
 * @tparam Domain The admissible element types.
 */
template <typename Domain = real_arithmetic_type_domain>
using binary_homogeneous_pair_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_same_as<0>, slot_same_as<0>>,
	type_list<slot_same_as<0>, slot_same_as<0>>
>;

/**
 * @brief Unary operation taking a boolean mask alongside its operand.
 *
 * The mask is an operand rather than a parameter, so it takes part in the
 * shape deduction like any other and may be broadcast against the operand
 * it selects from.
 *
 * @tparam Domain The admissible element types of the masked operand.
 */
template <typename Domain = arithmetic_type_domain>
using masked_unary_homogeneous_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_same_as<0>>,
	type_list<slot_same_as<0>, slot_fixed<numerical_type::boolean>>
>;

/**
 * @brief Binary operation taking a boolean mask alongside its operands.
 *
 * @tparam Domain The admissible element types of the masked operands.
 */
template <typename Domain = arithmetic_type_domain>
using masked_binary_homogeneous_rule = operand_type_rule<
	type_list<pivot_from_input<0, Domain>>,
	type_list<slot_same_as<0>>,
	type_list<
		slot_same_as<0>,
		slot_same_as<0>,
		slot_fixed<numerical_type::boolean>
	>
>;

/**
 * @brief Selection between two operands driven by a boolean condition.
 *
 * The element type follows the operands being selected from and not the
 * condition, which is why the pivot is taken from the second input rather
 * than the first.
 *
 * @tparam Domain The admissible element types of the selected operands.
 */
template <typename Domain = any_type_domain>
using selection_rule = operand_type_rule<
	type_list<pivot_from_input<1, Domain>>,
	type_list<slot_same_as<0>>,
	type_list<
		slot_fixed<numerical_type::boolean>,
		slot_same_as<0>,
		slot_same_as<0>
	>
>;

/**
 * @brief Binary operation whose operands have unrelated element types.
 *
 * Models operations combining values of two different kinds, such as
 * scaling a floating point value by an integer power of two. The output
 * follows the first operand.
 *
 * A backend dispatches the cartesian product of the two pivot domains, so
 * such a rule costs an order of magnitude more instantiations than a single
 * pivot one. Keep the second domain as narrow as the operation allows.
 *
 * @tparam FirstDomain The admissible element types of the first operand.
 * @tparam SecondDomain The admissible element types of the second operand.
 */
template <
	typename FirstDomain = floating_point_type_domain,
	typename SecondDomain = signed_integer_type_domain
>
using binary_heterogeneous_rule = operand_type_rule<
	type_list<
		pivot_from_input<0, FirstDomain>,
		pivot_from_input<1, SecondDomain>
	>,
	type_list<slot_same_as<0>>,
	type_list<slot_same_as<0>, slot_same_as<1>>
>;

/**
 * @brief Nullary operation whose output element type is free.
 *
 * With no input to fix it, the element type is whatever the user supplied
 * output carries. Models operations that write a value into an existing
 * array, such as `fill`.
 *
 * @tparam Domain The admissible output element types.
 */
template <typename Domain = any_type_domain>
using nullary_free_rule = operand_type_rule<
	type_list<pivot_from_output<0, Domain>>,
	type_list<slot_same_as<0>>,
	type_list<>
>;

/**
 * @brief Unary operation whose output element type is independent.
 *
 * The output adopts the input element type unless the user pre-allocates an
 * output of a different one, which is what makes a converting copy possible
 * without a separate operation.
 *
 * @tparam OutputDomain The admissible output element types.
 * @tparam InputDomain The admissible input element types.
 */
template <
	typename OutputDomain = any_type_domain,
	typename InputDomain = any_type_domain
>
using converting_rule = operand_type_rule<
	type_list<
		pivot_from_output<0, OutputDomain, 1>,
		pivot_from_input<0, InputDomain>
	>,
	type_list<slot_same_as<0>>,
	type_list<slot_same_as<1>>
>;

/** @} */

} // namespace ops
} // namespace rexlib
