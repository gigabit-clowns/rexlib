// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/em_component.hpp>
#include <rexlib/ops/parameters/axis_list.hpp>
#include <rexlib/ops/parameters/axis_selection.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

namespace rexlib
{
namespace em
{

REXLIB_DECLARE_PARAMETRIC_OPERATION_TRAITS(
	standardize_background,
	em_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value", "seed"),
	ops::keyed_axiswise_shape_rule,
	ops::axis_selection,
	ops::keyed_unary_homogeneous_rule<floating_point_type_domain>
);

/**
 * @brief Standardize images against the statistics of their background.
 *
 * An image is the part of the value along the named axes, and every other
 * axis is a batch axis. The centre of an image is the element at index
 * n / 2, rounded down, along each named axis. Its background holds the
 * elements whose squared distance from the centre, in elements, exceeds
 * the square of the radius. Images need not be square.
 *
 * Each image is standardized in three steps:
 * 1. The mean and the standard deviation of the finite background elements
 *    are computed. The standard deviation is that of the population.
 * 2. They are computed again, over the finite background elements whose
 *    standardized value lies within the bounds.
 * 3. Each element is standardized with the second statistics. An element
 *    that is not finite, or whose standardized value lies outside the
 *    bounds, is replaced by a draw from the standard normal distribution.
 *
 * Every element of the result is therefore either a standardized value
 * within the bounds or a draw. An image without a finite background
 * element has a mean of 0 and a standard deviation of 1. An image whose
 * standard deviation is 0 has its mean subtracted and is not scaled.
 *
 * The seed is a uint64 array without core axes. It broadcasts against the
 * batch axes of the value, so one seed may serve every image or each image
 * may have its own. The draw of an element depends only on its seed and on
 * its counter, which is the row-major position of the element among the
 * elements of the result sharing its seed. Images with seeds of their own
 * therefore draw the same values however they are batched. The draw for
 * counter c is the Box-Muller deviate of the Philox4x32-10 block at c / 2,
 * rounded down, keyed by the seed: its cosine branch when c is even, and
 * its sine branch when c is odd.
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API standardize_background_operation final
	: public parametric_operation<
		standardize_background_operation,
		standardize_background_operation_traits
	>
{
public:
	/**
	 * @brief Construct a standardization of the images along some axes.
	 *
	 * @param axes The axes of an image.
	 * @param radius The radius of the region excluded from the background,
	 * in elements.
	 * @param lower_bound The lowest standardized value kept, in standard
	 * deviations. Minus infinity keeps every value below the mean.
	 * @param upper_bound The highest standardized value kept, in standard
	 * deviations. Infinity keeps every value above the mean.
	 *
	 * @throws std::invalid_argument When an axis is repeated, when the
	 * radius is negative or NaN, or when the bounds do not lie on either
	 * side of 0.
	 */
	standardize_background_operation(
		ops::axis_list axes,
		double radius,
		double lower_bound,
		double upper_bound
	);

	/**
	 * @brief Get the radius of the region excluded from the background.
	 *
	 * @return double The radius, in elements.
	 */
	double get_radius() const noexcept;

	/**
	 * @brief Get the lowest standardized value kept.
	 *
	 * @return double The bound, in standard deviations.
	 */
	double get_lower_bound() const noexcept;

	/**
	 * @brief Get the highest standardized value kept.
	 *
	 * @return double The bound, in standard deviations.
	 */
	double get_upper_bound() const noexcept;

private:
	double m_radius;
	double m_lower_bound;
	double m_upper_bound;
};
REXLIB_END_TEMPLATE_BASE

} // namespace em
} // namespace rexlib
