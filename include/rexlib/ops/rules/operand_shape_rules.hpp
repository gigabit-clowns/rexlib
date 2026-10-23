// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "axes_of.hpp"
#include "count_of.hpp"
#include "extent_of.hpp"
#include "keep_dimensions_of.hpp"

#include <rexlib/core/dispatch/rules/core_dimensions.hpp>
#include <rexlib/core/dispatch/rules/named_core.hpp>
#include <rexlib/core/dispatch/rules/operand_axis_slot.hpp>
#include <rexlib/core/dispatch/rules/operand_extent.hpp>
#include <rexlib/core/dispatch/rules/operand_shape_rule.hpp>
#include <rexlib/core/dispatch/rules/output_batch.hpp>
#include <rexlib/core/dispatch/rules/positive_extent_domain.hpp>
#include <rexlib/core/dispatch/rules/trailing_core.hpp>
#include <rexlib/core/meta/type_list.hpp>

namespace rexlib
{
namespace ops
{

/**
 * @defgroup operand_shape_rules Common operand shape rules
 *
 * The shape rules the operation catalog is built from. An operation names
 * one of these in its declaration and that is the whole of its shape
 * contract. A rule says nothing about data types, so the same rule serves
 * operations typed in different ways.
 *
 * The rules are written without template arguments, so that a declaration
 * macro can take one as a single argument.
 *
 * @{
 */

/**
 * @brief Operation without operands to read that writes into the outputs it
 * is handed, whatever their shape.
 */
using nullary_elementwise_shape_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<>,
	trailing_core,
	output_batch::broadcast_target
>;

/**
 * @brief Operation of one operand applied to every element.
 */
using unary_elementwise_shape_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<>>,
	trailing_core,
	output_batch::broadcast_target
>;

/**
 * @brief Operation of two operands applied to every pair of elements their
 * shapes broadcast to.
 */
using binary_elementwise_shape_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<>, core_dimensions<>>,
	trailing_core,
	output_batch::broadcast_target
>;

/**
 * @brief Operation of three operands applied to every triple of elements
 * their shapes broadcast to.
 */
using ternary_elementwise_shape_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<>, core_dimensions<>, core_dimensions<>>,
	trailing_core,
	output_batch::broadcast_target
>;

/**
 * @brief Operation of one operand applied to every element, producing two
 * results of each.
 */
using unary_pair_elementwise_shape_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>, core_dimensions<>>,
	type_list<core_dimensions<>>,
	trailing_core,
	output_batch::broadcast_target
>;

/**
 * @brief Operation of two operands applied to every pair of elements,
 * producing two results of each.
 */
using binary_pair_elementwise_shape_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>, core_dimensions<>>,
	type_list<core_dimensions<>, core_dimensions<>>,
	trailing_core,
	output_batch::broadcast_target
>;

/**
 * @brief Reduction along the axes the operation names.
 *
 * The result drops the reduced axes, or keeps them with an extent of one.
 * Reducing an empty axis is admitted.
 */
using reduction_shape_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<axis_run<>>>,
	named_core<axes_of, keep_dimensions_of>
>;

/**
 * @brief Reduction selecting one element along the axes the operation
 * names.
 *
 * As reduction_shape_rule, except that every reduced axis must hold an
 * element to select.
 */
using extremum_reduction_shape_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<axis_run<positive_extent_domain>>>,
	named_core<axes_of, keep_dimensions_of>
>;

/**
 * @brief Dot product of two stacks of vectors along the axis the operation
 * names.
 *
 * The vectors must have the same length, which does not broadcast.
 */
using vector_dot_shape_rule = operand_shape_rule<
	type_list<extent_from_inputs<>>,
	type_list<core_dimensions<>>,
	type_list<
		core_dimensions<axis_same_as<0>>,
		core_dimensions<axis_same_as<0>>
	>,
	named_core<axes_of, keep_dimensions_of>
>;

/**
 * @brief Cross product of two stacks of three component vectors along the
 * axis the operation names.
 */
using cross_product_shape_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<axis_fixed<3>>>,
	type_list<
		core_dimensions<axis_fixed<3>>,
		core_dimensions<axis_fixed<3>>
	>,
	named_core<axes_of>
>;

/**
 * @brief Transform along the axes the operation names, each of which must
 * hold a sample.
 */
using fourier_transform_shape_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<axis_run<>>>,
	type_list<core_dimensions<axis_run<positive_extent_domain>>>,
	named_core<axes_of>
>;

/**
 * @brief Operation along the axes the operation names that keeps their
 * extents.
 */
using axiswise_shape_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<axis_run<>>>,
	type_list<core_dimensions<axis_run<>>>,
	named_core<axes_of>
>;

/**
 * @brief Transform of a real signal along the axes the operation names.
 *
 * The last axis named holds the n / 2 + 1 coefficients that are not
 * redundant, and every other keeps its extent.
 */
using real_fourier_transform_shape_rule = operand_shape_rule<
	type_list<extent_from_inputs<positive_extent_domain>>,
	type_list<core_dimensions<axis_run<>, axis_half_spectrum_of<0>>>,
	type_list<
		core_dimensions<axis_run<positive_extent_domain>, axis_same_as<0>>
	>,
	named_core<axes_of>
>;

/**
 * @brief Inverse of real_fourier_transform_shape_rule.
 *
 * The last axis named is restored to the extent the operation fixes, and
 * must hold the n / 2 + 1 coefficients of it.
 */
using inverse_real_fourier_transform_shape_rule = operand_shape_rule<
	type_list<extent_from_operation<extent_of, positive_extent_domain>>,
	type_list<core_dimensions<axis_run<>, axis_same_as<0>>>,
	type_list<
		core_dimensions<
			axis_run<positive_extent_domain>,
			axis_half_spectrum_of<0>
		>
	>,
	named_core<axes_of>
>;

/**
 * @brief Product of two stacks of matrices.
 */
using matrix_multiply_shape_rule = operand_shape_rule<
	type_list<
		extent_from_inputs<>,
		extent_from_inputs<>,
		extent_from_inputs<>
	>,
	type_list<core_dimensions<axis_same_as<0>, axis_same_as<2>>>,
	type_list<
		core_dimensions<axis_same_as<0>, axis_same_as<1>>,
		core_dimensions<axis_same_as<1>, axis_same_as<2>>
	>
>;

/**
 * @brief Product of a stack of matrices and a stack of vectors.
 */
using matrix_vector_shape_rule = operand_shape_rule<
	type_list<extent_from_inputs<>, extent_from_inputs<>>,
	type_list<core_dimensions<axis_same_as<0>>>,
	type_list<
		core_dimensions<axis_same_as<0>, axis_same_as<1>>,
		core_dimensions<axis_same_as<1>>
	>
>;

/**
 * @brief Product of a stack of vectors and a stack of matrices.
 */
using vector_matrix_shape_rule = operand_shape_rule<
	type_list<extent_from_inputs<>, extent_from_inputs<>>,
	type_list<core_dimensions<axis_same_as<1>>>,
	type_list<
		core_dimensions<axis_same_as<0>>,
		core_dimensions<axis_same_as<0>, axis_same_as<1>>
	>
>;

/**
 * @brief Sequence of as many values as the operation names.
 */
using sequence_shape_rule = operand_shape_rule<
	type_list<extent_from_operation<count_of>>,
	type_list<core_dimensions<axis_same_as<0>>>,
	type_list<>
>;

/** @} */

} // namespace ops
} // namespace rexlib
