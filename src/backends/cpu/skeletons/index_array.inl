// SPDX-License-Identifier: GPL-3.0-only

#include "index_array.hpp"

namespace rexlib
{
namespace cpu
{

template <std::size_t... Indices>
inline
std::array<std::size_t, sizeof...(Indices)> make_index_array(
	std::index_sequence<Indices...>
) noexcept
{
	return {{ Indices... }};
}

} // namespace cpu
} // namespace rexlib
