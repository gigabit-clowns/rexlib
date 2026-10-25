// SPDX-License-Identifier: GPL-3.0-only

#include "same_type_transform.hpp"

namespace rexlib
{

inline
const char* same_type_transform::get_relation() noexcept
{
	return "the same type as";
}

inline
numerical_type same_type_transform::apply(numerical_type type) noexcept
{
	return type;
}

} // namespace rexlib
