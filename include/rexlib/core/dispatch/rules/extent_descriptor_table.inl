// SPDX-License-Identifier: GPL-3.0-only

#include "extent_descriptor_table.hpp"

#include <array>

namespace rexlib
{

inline
span<const extent_descriptor>
extent_descriptor_table<type_list<>>::get() noexcept
{
	return span<const extent_descriptor>();
}

template <typename... Extents>
inline
span<const extent_descriptor>
extent_descriptor_table<type_list<Extents...>>::get() noexcept
{
	static const std::array<extent_descriptor, sizeof...(Extents)> table =
	{{
		extent_descriptor(
			Extents::source,
			make_extent_domain<typename Extents::domain_type>()
		)...
	}};

	return make_span(table.data(), table.size());
}

} // namespace rexlib
