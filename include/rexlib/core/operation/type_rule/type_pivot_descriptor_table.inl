// SPDX-License-Identifier: GPL-3.0-only

#include "type_pivot_descriptor_table.hpp"

#include <array>
#include <type_traits>

namespace rexlib
{
namespace detail
{

template <std::size_t Position, typename PivotList>
struct operation_pivot_position;

template <std::size_t Position>
struct operation_pivot_position<Position, type_list<>>
	: std::integral_constant<std::size_t, 0>
{
};

template <std::size_t Position, typename Head, typename... Tail>
struct operation_pivot_position<Position, type_list<Head, Tail...>>
	: std::integral_constant<
		std::size_t,
		Position == 0
			? 0
			: (Head::source == type_pivot_source::operation ? 1 : 0) +
				operation_pivot_position<
					(Position == 0 ? 0 : Position - 1),
					type_list<Tail...>
				>::value
	>
{
};

} // namespace detail

inline
span<const type_pivot_descriptor>
type_pivot_descriptor_table<type_list<>>::get() noexcept
{
	return span<const type_pivot_descriptor>();
}

template <typename... Pivots>
inline
span<const type_pivot_descriptor>
type_pivot_descriptor_table<type_list<Pivots...>>::get() noexcept
{
	return lower(std::index_sequence_for<Pivots...>());
}

template <typename... Pivots>
template <std::size_t... Positions>
inline
span<const type_pivot_descriptor>
type_pivot_descriptor_table<type_list<Pivots...>>::lower(
	std::index_sequence<Positions...>
) noexcept
{
	static const std::array<type_pivot_descriptor, sizeof...(Pivots)> table =
	{{
		type_pivot_descriptor(
			Pivots::source,
			Pivots::source == type_pivot_source::operation
				? detail::operation_pivot_position<
					Positions,
					type_list<Pivots...>
				>::value
				: Pivots::index,
			Pivots::default_pivot,
			Pivots::domain_type::get()
		)...
	}};

	return make_span(table.data(), table.size());
}

} // namespace rexlib
