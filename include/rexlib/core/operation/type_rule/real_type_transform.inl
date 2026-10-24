// SPDX-License-Identifier: GPL-3.0-only

#include "real_type_transform.hpp"

namespace rexlib
{

inline
const char* real_type_transform::get_relation() noexcept
{
	return "the real counterpart of";
}

inline
numerical_type real_type_transform::apply(numerical_type type) noexcept
{
	return make_real(type);
}

} // namespace rexlib
