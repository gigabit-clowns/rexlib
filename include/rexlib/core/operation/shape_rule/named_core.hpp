// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/core_placement.hpp>

#include <rexlib/core/platform/constexpr.hpp>

namespace rexlib
{

/**
 * @brief Placement of the core axes where the operation instance names
 * them.
 *
 * The axes are named in the frame of the broadcast inputs, whose rank is
 * the highest input rank. An output with no core axes either drops the
 * named axes or keeps them with an extent of one.
 *
 * @tparam AxesAccessor Type reading the named axes from the parameters of
 * the operation instance, through a static `get(const Parameters&)`
 * template returning `span<const std::size_t>`.
 * @tparam KeepAccessor Type reading whether an output keeps the named axes,
 * likewise returning `bool`. void when no output keeps them.
 *
 * @see operand_shape_rule
 */
template <typename AxesAccessor, typename KeepAccessor = void>
struct named_core
{
	static REXLIB_CONST_CONSTEXPR core_placement placement =
		core_placement::named;

	using axes_accessor_type = AxesAccessor;
	using keep_accessor_type = KeepAccessor;
};

} // namespace rexlib
