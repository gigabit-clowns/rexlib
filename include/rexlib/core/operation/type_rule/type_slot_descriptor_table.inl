// SPDX-License-Identifier: GPL-3.0-only

#include "type_slot_descriptor_table.hpp"

#include <array>

namespace rexlib
{

inline
span<const type_slot_descriptor>
type_slot_descriptor_table<type_list<>>::get() noexcept
{
	return span<const type_slot_descriptor>();
}

template <typename... Slots>
inline
span<const type_slot_descriptor>
type_slot_descriptor_table<type_list<Slots...>>::get() noexcept
{
	static const std::array<type_slot_descriptor, sizeof...(Slots)> table =
	{{
		type_slot_descriptor(
			Slots::pivot_index,
			make_type_transform<typename Slots::transform_type>()
		)...
	}};

	return make_span(table.data(), table.size());
}

} // namespace rexlib
