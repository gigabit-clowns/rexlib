// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/layout/joint_layout.hpp>

#include <type_traits>
#include <cstddef>
#include <tuple>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Tag representing an stride of 1.
 * 
 */
struct contiguous_stride_tag
	: std::integral_constant<std::ptrdiff_t, 1>
{
};

/**
 * @brief Tag representing an stride of 0.
 * 
 */
struct broadcasting_stride_tag
	: std::integral_constant<std::ptrdiff_t, 0>
{
};

/**
 * @brief Dispatch a function with potentially statically typed strides.
 * 
 * The callable will be invoked with a combination of `contiguous_stride_tag`,
 * `broadcasting_stride_tag` and std::ptrdiff_t for each operand.
 * 
 * @tparam F Type of the functor to be called. Must accept a tuple of `N`
 * integers or `std::integral_constant<std::ptrdiff_t, X>`.
 * @tparam N Statically defined number of operands in the array_access_layout.
 * @param callable The functor to be invoked.
 * @param layout The layout to be dispatched.
 * @param n_operands Tag defining the number of operands.
 * @return auto The result of invoking the callable with the provided layout.
 *
 * @warning The callable is instantiated for every combination of stride tags,
 * i.e. up to `3^N` specializations (broadcasting, contiguous or generic stride
 * per operand). Keep the number of operands small to avoid excessive compile
 * times and code bloat.
 */
template <typename F, std::size_t N>
auto dispatch_inner_loop_strides(
	F &&callable, 
	const joint_layout &layout,
	std::integral_constant<std::size_t, N> n_operands
);

/**
 * @brief Dispatch a function with potentially statically typed strides.
 * 
 * The callable will be invoked with a combination of `contiguous_stride_tag`,
 * `broadcasting_stride_tag` and std::ptrdiff_t for each operand.
 * 
 * @tparam F Type of the functor to be called. Must accept a tuple of
 * `sizeof...(Is)` integers or `std::integral_constant<std::ptrdiff_t, X>`.
 * @tparam Is Indices of operands at which strides are extracted from the
 * layout.
 * @param callable The functor to be invoked.
 * @param layout The layout to be dispatched.
 * @param operand_indices Tag indicating operand indices from which strides
 * are extracted.
 * @return auto The result of invoking the callable with the provided layout.
 *
 * @warning The callable is instantiated for every combination of stride tags,
 * i.e. up to `3^sizeof...(Is)` specializations (broadcasting, contiguous or
 * generic stride per operand). Keep the number of operands small to avoid
 * excessive compile times and code bloat.
 */
template <typename F, std::size_t... Is>
auto dispatch_inner_loop_strides(
	F &&callable, 
	const joint_layout &layout,
	std::index_sequence<Is...> operand_indices
);

} // namespace cpu
} // namespace rexlib

#include "inner_loop_stride_dispatch.inl"
