// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/indices/element_index_tags.hpp>

#include <cstddef>
#include <utility>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Take a run of the pointers a batch loop places at a batch element.
 *
 * @tparam Offset The position of the first pointer taken.
 * @tparam Pointers The tuple of the pointers.
 * @tparam Indices The positions taken, from @p Offset on.
 * @param pointers The pointers.
 * @return The pointers taken, as a tuple.
 */
template <std::size_t Offset, typename Pointers, std::size_t... Indices>
auto slice_pointers(
	const Pointers &pointers,
	std::index_sequence<Indices...>
);

/**
 * @brief Get the batch part of the counter a batch loop places last among
 * the pointers, as a batched_map_plan lays it out.
 *
 * @tparam Pointers The tuple of the pointers.
 * @param pointers The pointers, and the counter last for a linear index.
 * @param indexing The index of the map.
 * @return std::size_t The batch part of the counter, or 0 when the map is
 * handed no counter.
 */
template <typename Pointers>
std::size_t get_batch_counter(
	const Pointers &pointers,
	linear_index_tag indexing
) noexcept;

template <typename Pointers, typename Indexing>
std::size_t get_batch_counter(
	const Pointers &pointers,
	Indexing indexing
) noexcept;

} // namespace cpu
} // namespace rexlib

#include "batch_pointers.inl"
