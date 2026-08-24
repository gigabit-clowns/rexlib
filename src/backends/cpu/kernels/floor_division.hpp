// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cmath>
#include <type_traits>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Division that rounds towards negative infinity.
 *
 * C rounds a quotient towards zero, so -7 / 3 is -2 there. Rounding
 * towards negative infinity instead gives -3, which is what NumPy and
 * Python compute and what pairs with @ref floor_mod to satisfy
 * `floor_div(x, y) * y + floor_mod(x, y) == x` for either sign.
 *
 * @tparam T The element type. Unsigned types need no correction, their
 * quotient never being negative.
 * @param x The dividend.
 * @param y The divisor.
 * @return T The quotient, rounded down.
 */
template <typename T>
typename std::enable_if<
	std::is_integral<T>::value && std::is_unsigned<T>::value,
	T
>::type
floor_div(T x, T y)
{
	return static_cast<T>(x / y);
}

template <typename T>
typename std::enable_if<
	std::is_integral<T>::value && std::is_signed<T>::value,
	T
>::type
floor_div(T x, T y)
{
	const auto quotient = static_cast<T>(x / y);
	const auto remainder = static_cast<T>(x % y);

	// The truncated quotient is one too large exactly when the division
	// was inexact and the operands disagree in sign.
	return (remainder != T(0) && (x < T(0)) != (y < T(0)))
		? static_cast<T>(quotient - T(1))
		: quotient;
}

template <typename T>
typename std::enable_if<!std::is_integral<T>::value, T>::type
floor_div(T x, T y)
{
	using std::floor;
	return floor(x / y);
}

/**
 * @brief Give a remainder the sign of the divisor.
 *
 * @tparam T The element type.
 * @param remainder The remainder as C computes it.
 * @param divisor The divisor whose sign it must take.
 * @return T The corrected remainder.
 */
template <typename T>
T to_divisor_sign(T remainder, T divisor)
{
	return (remainder != T(0) && (remainder < T(0)) != (divisor < T(0)))
		? static_cast<T>(remainder + divisor)
		: remainder;
}

/**
 * @brief Remainder of a division that rounds towards negative infinity.
 *
 * Takes the sign of the divisor rather than of the dividend, so -7 % 3 is
 * 2 rather than C's -1. This is what NumPy and Python compute, and it is
 * the remainder that pairs with @ref floor_div.
 *
 * @tparam T The element type.
 * @param x The dividend.
 * @param y The divisor.
 * @return T The remainder.
 */
template <typename T>
typename std::enable_if<
	std::is_integral<T>::value && std::is_unsigned<T>::value,
	T
>::type
floor_mod(T x, T y)
{
	return static_cast<T>(x % y);
}

template <typename T>
typename std::enable_if<
	std::is_integral<T>::value && std::is_signed<T>::value,
	T
>::type
floor_mod(T x, T y)
{
	return to_divisor_sign(static_cast<T>(x % y), y);
}

template <typename T>
typename std::enable_if<!std::is_integral<T>::value, T>::type
floor_mod(T x, T y)
{
	using std::fmod;
	return to_divisor_sign(fmod(x, y), y);
}

} // namespace cpu
} // namespace rexlib
