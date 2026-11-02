// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/em_component.hpp>
#include <rexlib/em/ops/parameters/gridding_window.hpp>
#include <rexlib/em/ops/reconstruction/slice_insertion_rules.hpp>

namespace rexlib
{
namespace em
{

REXLIB_DECLARE_OPERATION_TRAITS(
	insert_weighted_slices,
	em_component,
	REXLIB_OPERANDS("numerator", "denominator"),
	REXLIB_OPERANDS(
		"numerator",
		"denominator",
		"slice",
		"weight",
		"transform",
		"radial_weight"
	),
	weighted_slice_insertion_shape_rule,
	weighted_slice_insertion_type_rule
);

/**
 * @brief Insert the spectra of projections into the spectrum of a volume,
 * with a denominator weight per frequency.
 *
 * Works as insert_slices_operation, with one more input after `slice`:
 * `weight`, a real half spectrum with the core axes (n, n / 2 + 1). With
 * W(k) the weight of the frequency k of a slice, the denominator receives
 * kappa(g - K) r W(k) instead of kappa(g - K) r. The frequency -k that an
 * element also stands for has the same weight. The numerator is unchanged,
 * so a weight it needs is multiplied into the slice beforehand.
 *
 * @see insert_slices_operation
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API insert_weighted_slices_operation final
	: public trivial_operation<
		insert_weighted_slices_operation,
		insert_weighted_slices_operation_traits
	>
{
public:
	/**
	 * @brief Construct an insertion with a gridding window.
	 *
	 * @param window The window that spreads a sample over the voxels.
	 */
	explicit insert_weighted_slices_operation(
		const gridding_window &window
	) noexcept;

	/**
	 * @brief Get the gridding window.
	 *
	 * @return const gridding_window& The window.
	 */
	const gridding_window& get_window() const noexcept;

private:
	gridding_window m_window;
};
REXLIB_END_TEMPLATE_BASE

} // namespace em
} // namespace rexlib
