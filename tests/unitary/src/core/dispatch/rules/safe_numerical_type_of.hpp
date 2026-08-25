// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>

#include <type_traits>

namespace rexlib
{
namespace test
{

/**
 * @brief Map a static type back to its numerical_type, or to unknown.
 *
 * numerical_type_of is deliberately undefined for types that have none,
 * such as std::complex<int>. Sweeping a transform or a rule over every
 * numerical type instantiates its type level half outside its domain too,
 * so detect the mapping rather than requiring it.
 *
 * @tparam T The static type.
 */
template <typename T, typename = void>
struct safe_numerical_type_of
	: std::integral_constant<numerical_type, numerical_type::unknown>
{
};

template <typename T>
struct safe_numerical_type_of<
	T,
	decltype(void(numerical_type_of<T>::value))
>
	: numerical_type_of<T>
{
};

} // namespace test
} // namespace rexlib
