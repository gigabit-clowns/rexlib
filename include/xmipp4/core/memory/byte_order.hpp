// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/attributes.hpp"
#include "../platform/constexpr.hpp"

#include <type_traits>

namespace rex
{

enum class byte_order 
{
	big_endian,
	little_endian,
	//pdp_endian, //unsupported
	//honeywell_endian, //unsupported
};

REX_CONSTEXPR byte_order get_system_byte_order() noexcept;
REX_CONSTEXPR byte_order get_fpu_byte_order() noexcept;

template<typename T>
REX_NODISCARD REX_CONSTEXPR 
typename std::enable_if<std::is_integral<T>::value, T>::type
reverse_byte_order(T x) noexcept;

template<typename T>
REX_CONSTEXPR T& reverse_byte_order_inplace(T& x) noexcept;

template<byte_order From, byte_order To, typename T>
REX_NODISCARD REX_CONSTEXPR 
typename std::enable_if<std::is_integral<T>::value, T>::type
convert_byte_order(T x) noexcept;

template<typename T>
REX_NODISCARD REX_CONSTEXPR 
typename std::enable_if<std::is_integral<T>::value, T>::type
convert_byte_order(T x, byte_order from, byte_order to) noexcept;

template<byte_order From, byte_order To, typename T>
REX_CONSTEXPR 
typename std::enable_if<std::is_integral<T>::value, T&>::type
convert_byte_order_inplace(T& x) noexcept;

template<typename T>
REX_CONSTEXPR 
typename std::enable_if<std::is_integral<T>::value, T&>::type
convert_byte_order_inplace(T& x, byte_order from, byte_order to) noexcept;

} // namespace rex

#include "byte_order.inl"
