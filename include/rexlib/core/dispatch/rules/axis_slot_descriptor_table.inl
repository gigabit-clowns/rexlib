// SPDX-License-Identifier: GPL-3.0-only

#include "axis_slot_descriptor_table.hpp"

#include "same_extent_transform.hpp"

#include <array>
#include <type_traits>

namespace rexlib
{

inline
span<const axis_slot_descriptor>
axis_slot_descriptor_table<type_list<>>::get() noexcept
{
	return span<const axis_slot_descriptor>();
}

template <typename... Slots>
inline
span<const axis_slot_descriptor>
axis_slot_descriptor_table<type_list<Slots...>>::get() noexcept
{
	static const std::array<axis_slot_descriptor, sizeof...(Slots)> table =
	{{
		axis_slot_descriptor(
			Slots::kind,
			Slots::index,
			&Slots::transform_type::apply,
			Slots::transform_type::get_relation(),
			std::is_same<
				typename Slots::transform_type,
				same_extent_transform
			>::value,
			make_extent_domain<typename Slots::domain_type>()
		)...
	}};

	return make_span(table.data(), table.size());
}

} // namespace rexlib
