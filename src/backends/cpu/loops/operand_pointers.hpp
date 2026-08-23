// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <utility>

namespace xmipp4
{
namespace cpu
{
namespace detail
{

/**
 * @brief Displace every pointer of a tuple by one offset each.
 *
 * @tparam Pointers CV-qualified operand pointers.
 * @tparam Is Indices of the operands, one per pointer.
 * @param pointers The pointers to displace.
 * @param offsets One offset per pointer, in element counts.
 * @param operand_indices Tag providing the indices.
 * @return std::tuple<Pointers...> The displaced pointers.
 */
template <typename... Pointers, std::size_t... Is>
std::tuple<Pointers...> offset_pointers(
	const std::tuple<Pointers...> &pointers,
	const std::ptrdiff_t *offsets,
	std::index_sequence<Is...> operand_indices
);

/**
 * @brief Advance every pointer of a tuple by a multiple of its own stride.
 *
 * The strides may be stride tags, in which case the multiplication is
 * resolved at compile time and a broadcast operand does not move at all.
 *
 * @tparam Pointers CV-qualified operand pointers.
 * @tparam Strides Stride of each operand. Either a stride tag or a runtime
 * `std::ptrdiff_t`.
 * @tparam Is Indices of the operands, one per pointer.
 * @param pointers The pointers to advance.
 * @param index How many steps to advance by.
 * @param strides The stride of each operand.
 * @param operand_indices Tag providing the indices.
 * @return std::tuple<Pointers...> The advanced pointers.
 */
template <typename... Pointers, typename... Strides, std::size_t... Is>
std::tuple<Pointers...> step_pointers(
	const std::tuple<Pointers...> &pointers,
	std::size_t index,
	const std::tuple<Strides...> &strides,
	std::index_sequence<Is...> operand_indices
);

/**
 * @brief Advance every pointer of a tuple by a multiple of its own stride,
 * the strides being read from an array.
 *
 * The overload the outputs take. They are left out of the stride dispatch, so
 * their strides are plain integers read once rather than tags.
 *
 * @tparam Pointers CV-qualified operand pointers.
 * @tparam Count Number of strides in the array. Must match the pointer count.
 * @tparam Is Indices of the operands, one per pointer.
 * @param pointers The pointers to advance.
 * @param index How many steps to advance by.
 * @param strides The stride of each operand.
 * @param operand_indices Tag providing the indices.
 * @return std::tuple<Pointers...> The advanced pointers.
 */
template <typename... Pointers, std::size_t Count, std::size_t... Is>
std::tuple<Pointers...> step_pointers(
	const std::tuple<Pointers...> &pointers,
	std::size_t index,
	const std::array<std::ptrdiff_t, Count> &strides,
	std::index_sequence<Is...> operand_indices
);

} // namespace detail
} // namespace cpu
} // namespace xmipp4

#include "operand_pointers.inl"
