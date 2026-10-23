// SPDX-License-Identifier: GPL-3.0-only

#include "core_dimensions_descriptor_table.hpp"

#include "axis_slot_descriptor_table.hpp"

#include <array>

namespace rexlib
{

inline
span<const core_dimensions_descriptor>
core_dimensions_descriptor_table<type_list<>>::get() noexcept
{
	return span<const core_dimensions_descriptor>();
}

template <typename... Cores>
inline
span<const core_dimensions_descriptor>
core_dimensions_descriptor_table<type_list<Cores...>>::get() noexcept
{
	static const std::array<core_dimensions_descriptor, sizeof...(Cores)>
	table =
	{{
		core_dimensions_descriptor(
			axis_slot_descriptor_table<typename Cores::slot_list>::get()
		)...
	}};

	return make_span(table.data(), table.size());
}

} // namespace rexlib
