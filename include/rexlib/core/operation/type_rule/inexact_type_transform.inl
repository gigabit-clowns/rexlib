// SPDX-License-Identifier: GPL-3.0-only

#include "inexact_type_transform.hpp"

namespace rexlib
{

inline
const char* inexact_type_transform::get_relation() noexcept
{
	return "the inexact counterpart of";
}

inline
numerical_type inexact_type_transform::apply(numerical_type type) noexcept
{
	return make_inexact(type);
}

} // namespace rexlib
