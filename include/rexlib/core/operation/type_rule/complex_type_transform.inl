// SPDX-License-Identifier: GPL-3.0-only

#include "complex_type_transform.hpp"

namespace rexlib
{

inline
const char* complex_type_transform::get_relation() noexcept
{
	return "the complex counterpart of";
}

inline
numerical_type complex_type_transform::apply(numerical_type type) noexcept
{
	return make_complex(type);
}

} // namespace rexlib
