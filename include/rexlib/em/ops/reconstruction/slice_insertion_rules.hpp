// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/operation/shape_rule/core_dimensions.hpp>
#include <rexlib/core/operation/shape_rule/extent_pivot.hpp>
#include <rexlib/core/operation/shape_rule/extent_slot.hpp>
#include <rexlib/core/operation/shape_rule/operand_shape_rule.hpp>
#include <rexlib/core/operation/shape_rule/output_batch.hpp>
#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>
#include <rexlib/core/operation/shape_rule/trailing_core.hpp>
#include <rexlib/core/operation/type_rule/operand_type_rule.hpp>
#include <rexlib/ops/rules/axis_half_spectrum_of.hpp>

namespace rexlib
{
namespace em
{

/**
 * @defgroup slice_insertion_rules Rules of the slice insertions
 *
 * The operands of an insertion of slices into the spectrum of a volume.
 * The extent variables are, in order: the extent N of the volume, the
 * extent n of a slice, 3 and 4. A volume spectrum has the core axes
 * (N, N, N / 2 + 1) and a slice (n, n / 2 + 1). The outputs accumulate
 * into the first two inputs.
 *
 * The element types are float32 or float64, shared by every operand. The
 * numerator and the slices hold the complex type.
 *
 * @{
 */

/**
 * @brief Core axes of the half spectrum of a cubic volume.
 */
using volume_half_spectrum_core = core_dimensions<
	axis_same_as<0>,
	axis_same_as<0>,
	ops::axis_half_spectrum_of<0>
>;

/**
 * @brief Core axes of the half spectrum of a square slice.
 */
using slice_half_spectrum_core = core_dimensions<
	axis_same_as<1>,
	ops::axis_half_spectrum_of<1>
>;

/**
 * @brief Core axes of a 3x4 transform.
 */
using slice_transform_core = core_dimensions<
	axis_same_as<2>,
	axis_same_as<3>
>;

/**
 * @brief Core axes of a weight per frequency shell of a slice.
 */
using slice_radial_core = core_dimensions<ops::axis_half_spectrum_of<1>>;

/**
 * @brief Core axes of the four CTF parameters of a slice.
 */
using slice_ctf_core = core_dimensions<axis_same_as<3>>;

/**
 * @brief Extent variables of the slice insertions.
 */
using slice_insertion_pivots = type_list<
	extent_from_inputs<positive_extent_domain>,
	extent_from_inputs<positive_extent_domain>,
	extent_fixed<3>,
	extent_fixed<4>
>;

/**
 * @brief Element types of the slice insertions.
 */
using slice_insertion_type_domain = domain_union<
	single_type_domain<numerical_type::float32>,
	single_type_domain<numerical_type::float64>
>;

/**
 * @brief Shape rule of insert_slices_operation.
 */
using slice_insertion_shape_rule = operand_shape_rule<
	slice_insertion_pivots,
	type_list<volume_half_spectrum_core, volume_half_spectrum_core>,
	type_list<
		volume_half_spectrum_core,
		volume_half_spectrum_core,
		slice_half_spectrum_core,
		slice_transform_core,
		slice_radial_core
	>,
	trailing_core,
	output_batch::accumulated
>;

/**
 * @brief Type rule of insert_slices_operation.
 */
using slice_insertion_type_rule = operand_type_rule<
	type_list<pivot_from_input<1, slice_insertion_type_domain>>,
	type_list<slot_complex_of<0>, slot_same_as<0>>,
	type_list<
		slot_complex_of<0>,
		slot_same_as<0>,
		slot_complex_of<0>,
		slot_same_as<0>,
		slot_same_as<0>
	>
>;

/**
 * @brief Shape rule of insert_weighted_slices_operation.
 */
using weighted_slice_insertion_shape_rule = operand_shape_rule<
	slice_insertion_pivots,
	type_list<volume_half_spectrum_core, volume_half_spectrum_core>,
	type_list<
		volume_half_spectrum_core,
		volume_half_spectrum_core,
		slice_half_spectrum_core,
		slice_half_spectrum_core,
		slice_transform_core,
		slice_radial_core
	>,
	trailing_core,
	output_batch::accumulated
>;

/**
 * @brief Type rule of insert_weighted_slices_operation.
 */
using weighted_slice_insertion_type_rule = operand_type_rule<
	type_list<pivot_from_input<1, slice_insertion_type_domain>>,
	type_list<slot_complex_of<0>, slot_same_as<0>>,
	type_list<
		slot_complex_of<0>,
		slot_same_as<0>,
		slot_complex_of<0>,
		slot_same_as<0>,
		slot_same_as<0>,
		slot_same_as<0>
	>
>;

/**
 * @brief Shape rule of insert_ctf_slices_operation.
 */
using ctf_slice_insertion_shape_rule = operand_shape_rule<
	slice_insertion_pivots,
	type_list<volume_half_spectrum_core, volume_half_spectrum_core>,
	type_list<
		volume_half_spectrum_core,
		volume_half_spectrum_core,
		slice_half_spectrum_core,
		slice_transform_core,
		slice_ctf_core,
		slice_radial_core
	>,
	trailing_core,
	output_batch::accumulated
>;

/**
 * @brief Type rule of insert_ctf_slices_operation.
 */
using ctf_slice_insertion_type_rule = operand_type_rule<
	type_list<pivot_from_input<1, slice_insertion_type_domain>>,
	type_list<slot_complex_of<0>, slot_same_as<0>>,
	type_list<
		slot_complex_of<0>,
		slot_same_as<0>,
		slot_complex_of<0>,
		slot_same_as<0>,
		slot_same_as<0>,
		slot_same_as<0>
	>
>;

/** @} */

} // namespace em
} // namespace rexlib
