// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/same_extent_transform.hpp>

#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Binding of one core axis to an extent variable through a
 * transform.
 *
 * @tparam PivotIndex Index of the extent variable the axis derives from.
 * @tparam Transform Transform applied to the extent of the variable.
 *
 * @see core_dimensions
 */
template <
	std::size_t PivotIndex,
	typename Transform = same_extent_transform
>
struct extent_slot
{
	static REXLIB_CONST_CONSTEXPR std::size_t pivot_index = PivotIndex;

	using transform_type = Transform;
};

/**
 * @brief Axis of the extent of an extent variable.
 *
 * @tparam E Index of the extent variable.
 */
template <std::size_t E>
using axis_same_as = extent_slot<E, same_extent_transform>;

} // namespace rexlib
