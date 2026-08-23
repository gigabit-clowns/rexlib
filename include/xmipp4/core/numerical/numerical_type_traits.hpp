// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "numerical_type.hpp"
#include "fixed_width_float.hpp"

#include <complex>

namespace rex
{

/**
 * @brief Convert static types to numerical_type.
 * 
 * The numerical_type of the provided static type is stored as 
 * numerical_type_of<T>::value
 * 
 * @tparam T The static type to be converted.
 */
template <typename T>
struct numerical_type_of;

template <>
struct numerical_type_of<bool>
	: std::integral_constant<numerical_type, numerical_type::boolean>
{
};

template <>
struct numerical_type_of<char>
	: std::integral_constant<numerical_type, numerical_type::char8>
{
};

template <>
struct numerical_type_of<std::int8_t>
	: std::integral_constant<numerical_type, numerical_type::int8>
{
};

template <>
struct numerical_type_of<std::uint8_t>
	: std::integral_constant<numerical_type, numerical_type::uint8>
{
};

template <>
struct numerical_type_of<std::int16_t>
	: std::integral_constant<numerical_type, numerical_type::int16>
{
};

template <>
struct numerical_type_of<std::uint16_t>
	: std::integral_constant<numerical_type, numerical_type::uint16>
{
};

template <>
struct numerical_type_of<std::int32_t>
	: std::integral_constant<numerical_type, numerical_type::int32>
{
};

template <>
struct numerical_type_of<std::uint32_t>
	: std::integral_constant<numerical_type, numerical_type::uint32>
{
};

template <>
struct numerical_type_of<std::int64_t>
	: std::integral_constant<numerical_type, numerical_type::int64>
{
};

template <>
struct numerical_type_of<std::uint64_t>
	: std::integral_constant<numerical_type, numerical_type::uint64>
{
};

template <>
struct numerical_type_of<float16_t>
	: std::integral_constant<numerical_type, numerical_type::float16>
{
};

template <>
struct numerical_type_of<float32_t>
	: std::integral_constant<numerical_type, numerical_type::float32>
{
};

template <>
struct numerical_type_of<float64_t>
	: std::integral_constant<numerical_type, numerical_type::float64>
{
};

template <>
struct numerical_type_of<std::complex<float16_t>>
	: std::integral_constant<numerical_type, numerical_type::complex_float16>
{
};

template <>
struct numerical_type_of<std::complex<float32_t>>
	: std::integral_constant<numerical_type, numerical_type::complex_float32>
{
};

template <>
struct numerical_type_of<std::complex<float64_t>>
	: std::integral_constant<numerical_type, numerical_type::complex_float64>
{
};



/**
 * @brief Convert numerical_type-s to static types.
 *
 * The static type of the provided numerical_type is exposed as
 * native_type_of<Type>::type.
 *
 * This is the inverse of numerical_type_of, so that
 * numerical_type_of<typename native_type_of<Type>::type>::value equals
 * Type for every convertible numerical_type.
 *
 * It is left undefined for numerical_type::unknown and
 * numerical_type::count, so that naming a type that has no static
 * counterpart is a compile time error.
 *
 * @tparam Type The numerical_type to be converted.
 *
 * @see numerical_type_of
 */
template <numerical_type Type>
struct native_type_of;

template <>
struct native_type_of<numerical_type::boolean>
{
	using type = bool;
};

template <>
struct native_type_of<numerical_type::char8>
{
	using type = char;
};

template <>
struct native_type_of<numerical_type::int8>
{
	using type = std::int8_t;
};

template <>
struct native_type_of<numerical_type::uint8>
{
	using type = std::uint8_t;
};

template <>
struct native_type_of<numerical_type::int16>
{
	using type = std::int16_t;
};

template <>
struct native_type_of<numerical_type::uint16>
{
	using type = std::uint16_t;
};

template <>
struct native_type_of<numerical_type::int32>
{
	using type = std::int32_t;
};

template <>
struct native_type_of<numerical_type::uint32>
{
	using type = std::uint32_t;
};

template <>
struct native_type_of<numerical_type::int64>
{
	using type = std::int64_t;
};

template <>
struct native_type_of<numerical_type::uint64>
{
	using type = std::uint64_t;
};

template <>
struct native_type_of<numerical_type::float16>
{
	using type = float16_t;
};

template <>
struct native_type_of<numerical_type::float32>
{
	using type = float32_t;
};

template <>
struct native_type_of<numerical_type::float64>
{
	using type = float64_t;
};

template <>
struct native_type_of<numerical_type::complex_float16>
{
	using type = std::complex<float16_t>;
};

template <>
struct native_type_of<numerical_type::complex_float32>
{
	using type = std::complex<float32_t>;
};

template <>
struct native_type_of<numerical_type::complex_float64>
{
	using type = std::complex<float64_t>;
};

} // namespace rex
