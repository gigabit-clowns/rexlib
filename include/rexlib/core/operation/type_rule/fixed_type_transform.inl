// SPDX-License-Identifier: GPL-3.0-only

#include "fixed_type_transform.hpp"

namespace rexlib
{

template <numerical_type Type>
inline
const char* fixed_type_transform<Type>::get_relation() noexcept
{
	return "independent of";
}

template <numerical_type Type>
inline
numerical_type fixed_type_transform<Type>::apply(numerical_type) noexcept
{
	return Type;
}

} // namespace rexlib
