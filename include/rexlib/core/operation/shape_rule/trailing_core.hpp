// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/core_placement.hpp>

#include <rexlib/core/platform/constexpr.hpp>

namespace rexlib
{

/**
 * @brief Placement of the core axes at the end of every operand.
 *
 * The axes before them are batch axes.
 *
 * @see operand_shape_rule
 */
struct trailing_core
{
	static REXLIB_CONST_CONSTEXPR core_placement placement =
		core_placement::trailing;
};

} // namespace rexlib
