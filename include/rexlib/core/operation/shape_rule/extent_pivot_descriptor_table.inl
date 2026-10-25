// SPDX-License-Identifier: GPL-3.0-only

#include "extent_pivot_descriptor_table.hpp"

#include <array>

namespace rexlib
{

inline
span<const extent_pivot_descriptor>
extent_pivot_descriptor_table<type_list<>>::get() noexcept
{
	return span<const extent_pivot_descriptor>();
}

template <typename... Extents>
inline
span<const extent_pivot_descriptor>
extent_pivot_descriptor_table<type_list<Extents...>>::get() noexcept
{
	static const std::array<extent_pivot_descriptor, sizeof...(Extents)> table =
	{{
		extent_pivot_descriptor(
			Extents::source,
			make_extent_domain<typename Extents::domain_type>()
		)...
	}};

	return make_span(table.data(), table.size());
}

} // namespace rexlib
