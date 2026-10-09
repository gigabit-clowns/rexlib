// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/attributes.hpp"
#include "../platform/constexpr.hpp"

#include <type_traits>
#include <cstdint>
#include <functional>
#include <iostream>

namespace rexlib
{

/**
 * @brief A byte of memory.
 *
 * An object of any type can be read and written through a pointer to
 * @c byte, as it can through a pointer to a character type. @c as_bytes
 * gives such a pointer.
 */
enum class REXLIB_MAY_ALIAS byte : uint8_t {};

REXLIB_CONSTEXPR uint8_t as_uint8(byte b) noexcept;
REXLIB_CONSTEXPR byte as_byte(uint8_t b) noexcept;

/**
 * @brief View an object as the bytes it is stored in.
 *
 * @param ptr Pointer to the object.
 * @return byte* Pointer to its first byte.
 */
template <typename T>
byte* as_bytes(T* ptr) noexcept;

/**
 * @brief View an object as the bytes it is stored in.
 *
 * @param ptr Pointer to the object.
 * @return const byte* Pointer to its first byte.
 */
template <typename T>
const byte* as_bytes(const T* ptr) noexcept;

template <class IntegerType>
REXLIB_CONSTEXPR byte operator<<(byte b, IntegerType shift) noexcept;
template <class IntegerType>
REXLIB_CONSTEXPR byte operator>>(byte b, IntegerType shift) noexcept;
template <class IntegerType>
REXLIB_CONSTEXPR byte& operator<<=(byte& b, IntegerType shift) noexcept;
template <class IntegerType>
REXLIB_CONSTEXPR byte& operator>>=(byte& b, IntegerType shift) noexcept;

REXLIB_CONSTEXPR byte operator~(byte b) noexcept;
REXLIB_CONSTEXPR byte operator|(byte lhs, byte rhs) noexcept;
REXLIB_CONSTEXPR byte operator&(byte lhs, byte rhs) noexcept;
REXLIB_CONSTEXPR byte operator^(byte lhs, byte rhs) noexcept;
REXLIB_CONSTEXPR byte& operator|=(byte& lhs, byte rhs) noexcept;
REXLIB_CONSTEXPR byte& operator&=(byte& lhs, byte rhs) noexcept;
REXLIB_CONSTEXPR byte& operator^=(byte& lhs, byte rhs) noexcept;

REXLIB_CONSTEXPR std::size_t get_byte_bits() noexcept;

template <typename C>
REXLIB_CONSTEXPR void to_hex(byte b, C &high, C &low) noexcept;

template<typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, const byte& b);

} // namespace rexlib

template <>
struct std::hash<rexlib::byte>
{
	REXLIB_CONSTEXPR size_t operator()(rexlib::byte b) const noexcept;
};

#include "byte.inl"
