// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../meta/type_tag.hpp"
#include "numerical_type.hpp"
#include "fixed_width_float.hpp"
#include "../platform/constexpr.hpp"

#include <complex>
#include <type_traits>

namespace rex
{

/**
 * @brief Dispatch a function with the numerical types.
 * 
 * This function dispatches a set of numerical_type-s and calls the provided
 * visitor with the dispatched types wrapped around a type_tag. In case a type
 * fails conversion to a C++ type, type_tag<void> is passed.
 * 
 * @tparam F Functor to be dispatched.
 * @tparam Types any amount of numeric_type-s
 * @param visitor Function to be called with the dispatched types.
 * @param types Types to be dispatched.
 * @return auto The result of calling visitor(types...)
 * 
 * @see numerical_type
 * @see type_tag
 */
template <typename F, typename... Types>
REXLIB_CONSTEXPR
auto dispatch_numerical_types(F&& visitor, Types&& ...types);


} // namespace rex

#include "numerical_type_dispatch.inl"
