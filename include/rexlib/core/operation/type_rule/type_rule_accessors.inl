// SPDX-License-Identifier: GPL-3.0-only

#include "type_rule_accessors.hpp"

namespace rexlib
{

namespace detail
{

template <typename Parameters, typename... Pivots>
inline
std::array<numerical_type, sizeof...(Pivots)> read_operation_pivots(
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
std::array<
	numerical_type,
	type_rule_accessors<Rule>::operation_pivot_count
>
type_rule_accessors<Rule>::get_operation_pivots(
	const Parameters &parameters
) noexcept
{
	return detail::read_operation_pivots(
		parameters,
		operation_pivot_list()
	);
}

} // namespace rexlib
