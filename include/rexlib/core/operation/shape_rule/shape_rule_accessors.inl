// SPDX-License-Identifier: GPL-3.0-only

#include "shape_rule_accessors.hpp"

namespace rexlib
{

namespace detail
{

template <typename Parameters>
inline
span<const std::size_t> read_named_axes(
	const Parameters&,
	trailing_core
) noexcept
{
	return span<const std::size_t>();
}

template <typename Parameters, typename AxesAccessor, typename KeepAccessor>
inline
span<const std::size_t> read_named_axes(
	const Parameters &parameters,
	named_core<AxesAccessor, KeepAccessor>
) noexcept
{
	return AxesAccessor::get(parameters);
}

template <typename Parameters>
inline
bool read_keep_dimensions(const Parameters&, trailing_core) noexcept
{
	return false;
}

template <typename Parameters, typename AxesAccessor>
inline
bool read_keep_dimensions(
	const Parameters&,
	named_core<AxesAccessor, void>
) noexcept
{
	return false;
}

template <typename Parameters, typename AxesAccessor, typename KeepAccessor>
inline
bool read_keep_dimensions(
	const Parameters &parameters,
	named_core<AxesAccessor, KeepAccessor>
) noexcept
{
	return KeepAccessor::get(parameters);
}

template <typename Parameters, typename... Extents>
inline
std::array<std::size_t, sizeof...(Extents)> read_operation_extents(
	const Parameters &parameters,
	type_list<Extents...>
) noexcept
{
	static_cast<void>(parameters);
	return {{ Extents::accessor_type::get(parameters)... }};
}

} // namespace detail

template <typename Rule>
template <typename Parameters>
inline
span<const std::size_t> shape_rule_accessors<Rule>::get_named_axes(
	const Parameters &parameters
) noexcept
{
	return detail::read_named_axes(
		parameters,
		typename Rule::placement_type()
	);
}

template <typename Rule>
template <typename Parameters>
inline
bool shape_rule_accessors<Rule>::get_keep_dimensions(
	const Parameters &parameters
) noexcept
{
	return detail::read_keep_dimensions(
		parameters,
		typename Rule::placement_type()
	);
}

template <typename Rule>
template <typename Parameters>
inline
typename shape_rule_accessors<Rule>::operation_extent_array_type
shape_rule_accessors<Rule>::get_operation_extents(
	const Parameters &parameters
) noexcept
{
	return detail::read_operation_extents(
		parameters,
		operation_extent_list_type()
	);
}

} // namespace rexlib
