// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "subscript_tags.hpp"

namespace rexlib 
{

REXLIB_INLINE_CONSTEXPR
ellipsis_tag ellipsis() noexcept
{
	return ellipsis_tag();
}

REXLIB_INLINE_CONSTEXPR
new_axis_tag new_axis() noexcept
{
	return new_axis_tag();
}

} // namespace rexlib
