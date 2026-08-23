// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "numerical_type_dispatch.hpp"

#include "fixed_width_float.hpp"

#include <complex>
#include <stdexcept>

namespace xmipp4
{

namespace detail
{

template <typename F, typename... Entries>
REX_INLINE_CONSTEXPR
auto dispatch_single_numerical_type(
	F&& visitor, 
	numerical_type type
) 
{
    switch (type) 
	{
	case numerical_type::boolean: 
		return std::forward<F>(visitor)(type_tag<bool>());
	case numerical_type::char8: 
		return std::forward<F>(visitor)(type_tag<char>());
	case numerical_type::int8: 
		return std::forward<F>(visitor)(type_tag<std::int8_t>());
	case numerical_type::uint8:
		return std::forward<F>(visitor)(type_tag<std::uint8_t>());
	case numerical_type::int16:
		return std::forward<F>(visitor)(type_tag<std::int16_t>());
	case numerical_type::uint16:
		return std::forward<F>(visitor)(type_tag<std::uint16_t>());
	case numerical_type::int32:
		return std::forward<F>(visitor)(type_tag<std::int32_t>());
	case numerical_type::uint32:
		return std::forward<F>(visitor)(type_tag<std::uint32_t>());
	case numerical_type::int64:
		return std::forward<F>(visitor)(type_tag<std::int64_t>());
	case numerical_type::uint64:
		return std::forward<F>(visitor)(type_tag<std::uint64_t>());
	case numerical_type::float16:
		return std::forward<F>(visitor)(type_tag<float16_t>());
	case numerical_type::float32:
		return std::forward<F>(visitor)(type_tag<float32_t>());
	case numerical_type::float64: 
		return std::forward<F>(visitor)(type_tag<float64_t>());
	case numerical_type::complex_float16:
		return std::forward<F>(visitor)(type_tag<std::complex<float16_t>>());
	case numerical_type::complex_float32:
		return std::forward<F>(visitor)(type_tag<std::complex<float32_t>>());
	case numerical_type::complex_float64:
		return std::forward<F>(visitor)(type_tag<std::complex<float64_t>>());
	default: 
		throw std::invalid_argument("Unknown numerical type");
    }
}

template <typename F>
REX_INLINE_CONSTEXPR
auto dispatch_numerical_types(F&& visitor) 
{
    return std::forward<F>(visitor)();
}

template <typename F, typename... Types>
REX_INLINE_CONSTEXPR
auto dispatch_numerical_types(
	F&& visitor, 
	numerical_type type, 
	Types ...other_types
) 
{
    return dispatch_single_numerical_type(
		[&visitor, &other_types...](auto tag) 
		{
			const auto bound_visitor = [&](auto... other_tags) 
			{
   				return std::forward<F>(visitor)(tag, other_tags...);
        	};

        	return dispatch_numerical_types(
				bound_visitor,
				other_types...
			);
   		},
		type
	);
}

} // namespace detail

template <typename F, typename... Types>
REX_INLINE_CONSTEXPR
auto dispatch_numerical_types(
	F&& visitor, 
	Types&& ...types
)
{
	return detail::dispatch_numerical_types(
		std::forward<F>(visitor), 
		types...
	);
}

} // namespace xmipp4
