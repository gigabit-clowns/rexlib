// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../numerical/fixed_width_float.hpp"
#include "../platform/attributes.hpp"
#include "../platform/constexpr.hpp"

#include <complex>
#include <type_traits>

namespace rexlib
{

enum class byte_order 
{
	big_endian,
	little_endian,
	//pdp_endian, //unsupported
	//honeywell_endian, //unsupported
};

REXLIB_CONSTEXPR byte_order get_system_byte_order() noexcept;
REXLIB_CONSTEXPR byte_order get_fpu_byte_order() noexcept;

template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_integral<T>::value, T>::type
reverse_byte_order(T x) noexcept;

/**
 * @brief Reverse the bytes of a single precision value.
 *
 * @param x The value.
 * @return float The value with its bytes in the other order.
 */
REXLIB_NODISCARD float reverse_byte_order(float x) noexcept;

/**
 * @brief Reverse the bytes of a double precision value.
 *
 * @param x The value.
 * @return double The value with its bytes in the other order.
 */
REXLIB_NODISCARD double reverse_byte_order(double x) noexcept;

/**
 * @brief Reverse the bytes of a half precision value.
 *
 * @param x The value.
 * @return float16_t The value with its bytes in the other order.
 */
REXLIB_NODISCARD float16_t reverse_byte_order(float16_t x) noexcept;

/**
 * @brief Reverse the bytes of each component of a complex value.
 *
 * The real and the imaginary parts are reversed each on its own, rather than
 * the whole value at once, which would also exchange them.
 *
 * @tparam T The component type.
 * @param x The value.
 * @return std::complex<T> The value with the bytes of each component in the
 * other order.
 */
template<typename T>
REXLIB_NODISCARD
std::complex<T> reverse_byte_order(const std::complex<T> &x) noexcept;

template<typename T>
REXLIB_CONSTEXPR T& reverse_byte_order_inplace(T& x) noexcept;

template<byte_order From, byte_order To, typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_integral<T>::value, T>::type
convert_byte_order(T x) noexcept;

template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_integral<T>::value, T>::type
convert_byte_order(T x, byte_order from, byte_order to) noexcept;

template<byte_order From, byte_order To, typename T>
REXLIB_CONSTEXPR 
typename std::enable_if<std::is_integral<T>::value, T&>::type
convert_byte_order_inplace(T& x) noexcept;

template<typename T>
REXLIB_CONSTEXPR 
typename std::enable_if<std::is_integral<T>::value, T&>::type
convert_byte_order_inplace(T& x, byte_order from, byte_order to) noexcept;

} // namespace rexlib

#include "byte_order.inl"
