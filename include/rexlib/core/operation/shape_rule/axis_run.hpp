// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/any_extent_domain.hpp>

namespace rexlib
{

/**
 * @brief Leading core axes as many as the operation instance names.
 *
 * Every operand carrying the run has the same extents along it. It may only
 * lead core_dimensions, and only under named placement.
 *
 * @tparam Domain Tag naming the extents every axis of the run may take.
 *
 * @see core_dimensions
 */
template <typename Domain = any_extent_domain>
struct axis_run
{
	using domain_type = Domain;
};

} // namespace rexlib
