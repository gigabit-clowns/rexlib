// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/constexpr.hpp"
#include "../platform/attributes.hpp"

#include <cstddef>
#include <type_traits>

namespace rex
{

/**
 * @brief Returns a std::size_t type with a single bit set at the
 * provided index.
 * 
 * If the provided index is larger than the width of std::size_t,
 * the result will be zero.
 * 
 * @param i Index of the bit to be set. Zero based, 
 * starting from the LSB
 * @return std::size_t with one or zero bits set.
 */
REXLIB_NODISCARD REXLIB_CONSTEXPR 
std::size_t bit(std::size_t i) noexcept;

/**
 * @brief 
 * 
 * @tparam T Unsigned integer type
 * @param start First bit in the range (0 is LSB)
 * @param end Past-the-end bit in the range (0 is LSB)
 * @return T The mask
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
bit_range_mask(std::size_t start, std::size_t end) noexcept;

/**
 * @brief Sets the lowest '0' bit to '1' in place
 * 
 * When there are no bits to be set (because
 * all are '1'-s) it does not have any effect
 * 
 * @tparam T unsigned type
 * @param x The input/output word
 * @return A reference to x
 */
template<typename T>
REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T&>::type
set_lowest_zero_inplace(T& x) noexcept;

/**
 * @brief Sets the lowest '1' bit to '0' in place
 * 
 * When there are no bits to be cleared (because
 * all are '0'-s) it does not have any effect
 * 
 * @tparam T unsigned type
 * @param x The input/output word
 * @return A reference to x
 */
template<typename T>
REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T&>::type
clear_lowest_one_inplace(T& x) noexcept;

/**
 * @brief Returns a mask for all the consecutive trailing '0'-s
 * of the input word
 * 
 * Examples:
 * 0b00000111 -> 0b00000000\n 
 * 0b00000110 -> 0b00000001\n 
 * 0b00000100 -> 0b00000011\n 
 * 0b00000000 -> 0b11111111\n 
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @return The mask
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
mask_trailing_zeros(T x) noexcept;

/**
 * @brief Returns a mask for all the consecutive trailing '1'-s
 * of the input word
 * 
 * Examples:
 * 0b00000111 -> 0b00000111\n 
 * 0b00000110 -> 0b00000000\n 
 * 0b00000101 -> 0b00000001\n 
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @return The mask
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
mask_trailing_ones(T x) noexcept;

/**
 * @brief Returns the input word with a bitwise rotation towards the left
 * 
 * If the bit count is negative, it is equivalent to calling
 * rotate_right(x, -s).
 * 
 * If the bit count is larger than the size of the word, it
 * repeats cyclically.
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @param s Number of bits to be shifted
 * @return Input word rotated left
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
rotate_left(T x, int s) noexcept;

/**
 * @brief Returns the input word with a bitwise rotation towards the right
 * 
 * If the bit count is negative, it is equivalent to calling
 * rotate_left(x, -s)
 * 
 * If the bit count is larger than the size of the word, it
 * repeats cyclically.
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @param s Number of bits to be shifted
 * @return Input word rotated right
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
rotate_right(T x, int s) noexcept;

/**
 * @brief Checks if the input word has one and only one bit set to '1'
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @return true when x has a single bit set, otherwise false
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, bool>::type
has_single_bit(T x) noexcept;

/**
 * @brief Returns the largest power of 2 not greater than the input word
 * 
 * If the input word is zero, it returns zero
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @return The result
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
bit_floor(T x) noexcept;

/**
 * @brief Returns the smallest power of 2 not less than the input word
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @return The result 
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
bit_ceil(T x) noexcept;

/**
 * @brief Returns the number of bits required to represent the input number
 * 
 * @tparam T unsigned type
 * @param x The input number
 * @return The number of bits required to represent the input
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
bit_width(T x) noexcept;

/**
 * @brief Counts the number of consecutive '0's from the MSB
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @return The number of consecutive '0's in the input 
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
count_leading_zeros(T x) noexcept;

/**
 * @brief Counts the number of consecutive '1's from the MSB
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @return The number of consecutive '1's in the input 
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
count_leading_ones(T x) noexcept;

/**
 * @brief Counts the number of consecutive '0's from the LSB
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @return The number of consecutive '0's in the input 
 */
template<typename T>
REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
count_trailing_zeros(T x) noexcept;

/**
 * @brief Counts the number of consecutive '1's from the LSB
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @return The number of consecutive '1's in the input 
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
count_trailing_ones(T x) noexcept;

/**
 * @brief Counts the number '1's in the input word
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @return The number of '1's in the input 
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
popcount(T x) noexcept;

/**
 * @brief Computes the parity of the input word
 * 
 * When the number of '1's in the input word is odd
 * it returns true. Otherwise it returns false.
 * 
 * @tparam T unsigned type
 * @param x The input word
 * @return The number of '1's in the input 
 */
template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, bool>::type
parity(T x) noexcept;

} // namespace rex

#include "bit.inl"
