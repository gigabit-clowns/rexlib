// SPDX-License-Identifier: GPL-3.0-only

#include "type_rule_accessors.hpp"

namespace rexlib
{

namespace detail
{

template <typename Parameters, typename... Pivots>
inline
std::array<numerical_type, sizeof...(Pivots)> read_operation_data_types(
	const Parameters &parameters,
	type_list<Pivots...>
) noexcept
{
	static_cast<void>(parameters);
	return {{ Pivots::accessor_type::get(parameters)... }};
}

} // namespace detail

template <typename Rule>
template <typename Parameters>
inline
typename type_rule_accessors<Rule>::operation_data_type_array_type
type_rule_accessors<Rule>::get_operation_data_types(
	const Parameters &parameters
) noexcept
{
	return detail::read_operation_data_types(
		parameters,
		operation_pivot_list_type()
	);
}

} // namespace rexlib
