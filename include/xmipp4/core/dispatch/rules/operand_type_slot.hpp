// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operand_type_transform.hpp"

#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/platform/constexpr.hpp>

#include <cstddef>

namespace rex
{

/**
 * @brief Binding of one operand to a pivot through a transform.
 *
 * A typing rule declares one slot per operand, in signature order. The slot
 * says which free variable the operand follows and how.
 *
 * @tparam PivotIndex Index of the pivot this operand derives from.
 * @tparam Transform Transform applied to the pivot type.
 *
 * @see operand_type_rule
 * @see operand_type_transform
 */
template <std::size_t PivotIndex, typename Transform = same_type_transform>
struct operand_type_slot
{
	static REX_CONST_CONSTEXPR std::size_t pivot_index = PivotIndex;

	using transform_type = Transform;
};

/**
 * @brief Slot whose operand has the pivot type itself.
 *
 * @tparam I Index of the pivot.
 */
template <std::size_t I = 0>
using slot_same_as = operand_type_slot<I, same_type_transform>;

/**
 * @brief Slot whose operand has the real counterpart of the pivot type.
 *
 * @tparam I Index of the pivot.
 */
template <std::size_t I = 0>
using slot_real_of = operand_type_slot<I, real_type_transform>;

/**
 * @brief Slot whose operand has the complex counterpart of the pivot type.
 *
 * @tparam I Index of the pivot.
 */
template <std::size_t I = 0>
using slot_complex_of = operand_type_slot<I, complex_type_transform>;

/**
 * @brief Slot whose operand has the inexact counterpart of the pivot type.
 *
 * @tparam I Index of the pivot.
 */
template <std::size_t I = 0>
using slot_inexact_of = operand_type_slot<I, inexact_type_transform>;

/**
 * @brief Slot whose operand has a fixed type, whatever the pivot is.
 *
 * @tparam Type The type of the operand.
 * @tparam I Index of the pivot. Unused, but kept so that every slot names
 * a pivot.
 */
template <numerical_type Type, std::size_t I = 0>
using slot_fixed = operand_type_slot<I, fixed_type_transform<Type>>;

} // namespace rex
