// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "any_extent_domain.hpp"
#include "half_spectrum_extent_transform.hpp"
#include "same_extent_transform.hpp"

#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief How a core axis of an operand gets its extent.
 */
enum class axis_slot_kind
{
	/**
	 * @brief From an extent variable, through a transform.
	 */
	extent,

	/**
	 * @brief A fixed extent, whatever the operands are.
	 */
	fixed,

	/**
	 * @brief Zero or more axes, as many as the operation instance names,
	 * matched position by position across the operands that carry them.
	 */
	run
};

/**
 * @brief Declaration of one core axis of an operand.
 *
 * A shape rule declares one slot per core axis of every operand, in the
 * order the axes are laid out.
 *
 * @tparam Kind How the axis gets its extent.
 * @tparam Index The index of the extent variable when @p Kind is
 * axis_slot_kind::extent, the extent itself when it is
 * axis_slot_kind::fixed, and unused otherwise.
 * @tparam Transform Transform applied to the extent variable. Only
 * meaningful when @p Kind is axis_slot_kind::extent.
 * @tparam Domain Tag naming the extents the axes of a run may take. Only
 * meaningful when @p Kind is axis_slot_kind::run.
 *
 * @see core_dimensions
 */
template <
	axis_slot_kind Kind,
	std::size_t Index,
	typename Transform,
	typename Domain
>
struct operand_axis_slot
{
	static REXLIB_CONST_CONSTEXPR axis_slot_kind kind = Kind;
	static REXLIB_CONST_CONSTEXPR std::size_t index = Index;

	using transform_type = Transform;
	using domain_type = Domain;
};

/**
 * @brief Axis whose extent is that of an extent variable.
 *
 * @tparam E Index of the extent variable.
 */
template <std::size_t E>
using axis_same_as = operand_axis_slot<
	axis_slot_kind::extent,
	E,
	same_extent_transform,
	any_extent_domain
>;

/**
 * @brief Axis holding the half spectrum of an extent variable.
 *
 * @tparam E Index of the extent variable.
 */
template <std::size_t E>
using axis_half_spectrum_of = operand_axis_slot<
	axis_slot_kind::extent,
	E,
	half_spectrum_extent_transform,
	any_extent_domain
>;

/**
 * @brief Axis of a fixed extent.
 *
 * @tparam N The extent.
 */
template <std::size_t N>
using axis_fixed = operand_axis_slot<
	axis_slot_kind::fixed,
	N,
	same_extent_transform,
	any_extent_domain
>;

/**
 * @brief Zero or more axes, as many as the operation instance names.
 *
 * @tparam Domain Tag naming the extents the axes may take.
 */
template <typename Domain = any_extent_domain>
using axis_run = operand_axis_slot<
	axis_slot_kind::run,
	0,
	same_extent_transform,
	Domain
>;

} // namespace rexlib
