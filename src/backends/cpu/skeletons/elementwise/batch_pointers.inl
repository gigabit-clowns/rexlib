// SPDX-License-Identifier: GPL-3.0-only

#include "batch_pointers.hpp"

#include <tuple>

namespace rexlib
{
namespace cpu
{

template <std::size_t Offset, typename Pointers, std::size_t... Indices>
inline
auto slice_pointers(
	const Pointers &pointers,
	std::index_sequence<Indices...>
)
{
	return std::make_tuple(std::get<Offset + Indices>(pointers)...);
}

template <typename Pointers>
inline
std::size_t get_batch_counter(
	const Pointers &pointers,
	linear_index_tag /*indexing*/
) noexcept
{
	return std::get<std::tuple_size<Pointers>::value - 1>(pointers);
}

template <typename Pointers, typename Indexing>
inline
std::size_t get_batch_counter(
	const Pointers& /*pointers*/,
	Indexing /*indexing*/
) noexcept
{
	return 0;
}

} // namespace cpu
} // namespace rexlib
