// SPDX-License-Identifier: GPL-3.0-only

#include "extent_slot_descriptor_table.hpp"

#include <array>

namespace rexlib
{

inline
span<const extent_slot_descriptor>
extent_slot_descriptor_table<type_list<>>::get() noexcept
{
	return span<const extent_slot_descriptor>();
}

template <typename... Slots>
inline
span<const extent_slot_descriptor>
extent_slot_descriptor_table<type_list<Slots...>>::get() noexcept
{
	static const std::array<extent_slot_descriptor, sizeof...(Slots)> table =
	{{
		extent_slot_descriptor(
			Slots::pivot_index,
			make_extent_transform<typename Slots::transform_type>()
		)...
	}};

	return make_span(table.data(), table.size());
}

} // namespace rexlib
