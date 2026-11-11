// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cmath>
#include <type_traits>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The larger of two elements, an unordered one winning.
 *
 * std::max and std::fmax both answer for a not-a-number, and neither
 * answers the way NumPy does. std::max returns whichever operand the
 * comparison happens to favour, so it propagates a not-a-number from one
 * side and swallows it from the other; std::fmax deliberately quiets it
 * and returns the other operand. NumPy propagates it from either side,
 * which is the only answer that keeps the operation commutative, and is
 * what this gives.
 *
 * @tparam T The element type. Types that cannot hold a not-a-number take
 * the plain comparison.
 * @param x The first element.
 * @param y The second element.
 * @return T The larger, or whichever is not a number.
 */
template <typename T>
inline
typename std::enable_if<!std::is_floating_point<T>::value, T>::type
maximum_of(T x, T y) noexcept
{
	return x < y ? y : x;
}

template <typename T>
inline
typename std::enable_if<std::is_floating_point<T>::value, T>::type
maximum_of(T x, T y) noexcept
{
	using std::isnan;

	if (isnan(x))
	{
		return x;
	}
	if (isnan(y))
	{
		return y;
	}

	return x < y ? y : x;
}

inline
bool maximum_of(bool x, bool y) noexcept
{
	return x || y;
}

/**
 * @brief The smaller of two elements, an unordered one winning.
 *
 * @see maximum_of
 *
 * @tparam T The element type.
 * @param x The first element.
 * @param y The second element.
 * @return T The smaller, or whichever is not a number.
 */
template <typename T>
inline
typename std::enable_if<!std::is_floating_point<T>::value, T>::type
minimum_of(T x, T y) noexcept
{
	return y < x ? y : x;
}

template <typename T>
inline
typename std::enable_if<std::is_floating_point<T>::value, T>::type
minimum_of(T x, T y) noexcept
{
	using std::isnan;

	if (isnan(x))
	{
		return x;
	}
	if (isnan(y))
	{
		return y;
	}

	return y < x ? y : x;
}

inline
bool minimum_of(bool x, bool y) noexcept
{
	return x && y;
}

/**
 * @brief Whether an element should displace the current largest.
 *
 * The comparison behind @ref maximum_of, answering with which of the two won
 * rather than with the winner itself, which is what an operation reporting a
 * position needs. Strict, so that an equal element does not displace the one
 * already held and a tie keeps the first place it was seen.
 *
 * A not-a-number displaces any number and nothing displaces it, so the first
 * one encountered is the one reported. That is what NumPy's argmax answers,
 * and it is the same rule @ref maximum_of follows.
 *
 * @tparam T The element type. Types that cannot hold a not-a-number take the
 * plain comparison.
 * @param x The candidate element.
 * @param y The element currently held.
 * @return bool True if @p x should displace @p y.
 */
template <typename T>
inline
typename std::enable_if<!std::is_floating_point<T>::value, bool>::type
exceeds(T x, T y) noexcept
{
	return y < x;
}

template <typename T>
inline
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
exceeds(T x, T y) noexcept
{
	using std::isnan;

	if (isnan(x))
	{
		return !isnan(y);
	}
	if (isnan(y))
	{
		return false;
	}

	return y < x;
}

/**
 * @brief Whether a boolean should displace the current largest.
 *
 * Spelled as a conjunction rather than a comparison, which is the only way a
 * boolean holds the answer and what keeps the comparison warning-free.
 *
 * @param x The candidate element.
 * @param y The element currently held.
 * @return bool True if @p x is set and @p y is not.
 */
inline
bool exceeds(bool x, bool y) noexcept
{
	return x && !y;
}

template <typename T>
inline
typename std::enable_if<!std::is_floating_point<T>::value, bool>::type
precedes(T x, T y) noexcept
{
	return x < y;
}

template <typename T>
inline
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
precedes(T x, T y) noexcept
{
	using std::isnan;

	if (isnan(x))
	{
		return !isnan(y);
	}
	if (isnan(y))
	{
		return false;
	}

	return x < y;
}

inline
bool precedes(bool x, bool y) noexcept
{
	return !x && y;
}

} // namespace cpu
} // namespace rexlib
