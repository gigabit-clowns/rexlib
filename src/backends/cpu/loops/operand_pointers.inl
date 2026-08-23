// SPDX-License-Identifier: GPL-3.0-only

#include "operand_pointers.hpp"

namespace xmipp4
{
namespace cpu
{
namespace detail
{

template <typename... Pointers, std::size_t... Is>
inline
std::tuple<Pointers...> offset_pointers(
	const std::tuple<Pointers...> &pointers,
	const std::ptrdiff_t *offsets,
	std::index_sequence<Is...>
)
{
	return std::make_tuple((std::get<Is>(pointers) + offsets[Is])...);
}

template <typename... Pointers, typename... Strides, std::size_t... Is>
inline
std::tuple<Pointers...> step_pointers(
	const std::tuple<Pointers...> &pointers,
	std::size_t index,
	const std::tuple<Strides...> &strides,
	std::index_sequence<Is...>
)
{
	const auto step = static_cast<std::ptrdiff_t>(index);
	return std::make_tuple(
		(
			std::get<Is>(pointers) +
			step * static_cast<std::ptrdiff_t>(std::get<Is>(strides))
		)...
	);
}

template <typename... Pointers, std::size_t Count, std::size_t... Is>
inline
std::tuple<Pointers...> step_pointers(
	const std::tuple<Pointers...> &pointers,
	std::size_t index,
	const std::array<std::ptrdiff_t, Count> &strides,
	std::index_sequence<Is...>
)
{
	const auto step = static_cast<std::ptrdiff_t>(index);
	return std::make_tuple(
		(std::get<Is>(pointers) + step * strides[Is])...
	);
}

} // namespace detail
} // namespace cpu
} // namespace xmipp4
