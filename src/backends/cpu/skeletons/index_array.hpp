// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <array>
#include <cstddef>
#include <utility>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Make an array of the indices of an index sequence.
 *
 * @tparam Indices The indices.
 * @return std::array<std::size_t, sizeof...(Indices)> The indices, in
 * order.
 */
template <std::size_t... Indices>
std::array<std::size_t, sizeof...(Indices)> make_index_array(
	std::index_sequence<Indices...>
) noexcept;

} // namespace cpu
} // namespace rexlib

#include "index_array.inl"
