// SPDX-License-Identifier: GPL-3.0-only

#include "operation_data_types.hpp"

#include <type_traits>

namespace rexlib
{

namespace detail
{

template <typename Op>
inline
std::array<numerical_type, 0> read_instance_data_types(
	const Op&,
	std::true_type /*none*/
) noexcept
{
	return {};
}

template <typename Op>
inline
std::array<
	numerical_type,
	type_rule_accessors<typename Op::type_rule>::operation_pivot_count
>
read_instance_data_types(const Op &operation, std::false_type /*none*/) noexcept
{
	return type_rule_accessors<
		typename Op::type_rule
	>::get_operation_data_types(operation.get_parameters());
}

} // namespace detail

template <typename Op>
inline
std::array<
	numerical_type,
	type_rule_accessors<typename Op::type_rule>::operation_pivot_count
>
get_operation_data_types(const Op &operation) noexcept
{
	using none = std::integral_constant<
		bool,
		type_rule_accessors<
			typename Op::type_rule
		>::operation_pivot_count == 0
	>;
	return detail::read_instance_data_types(operation, none());
}

} // namespace rexlib
