// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/attributes.hpp"
#include "../platform/constexpr.hpp"

#include <type_traits>
#include <cstdint>
#include <functional>
#include <iostream>

namespace rex
{

enum class byte : uint8_t {};

REX_CONSTEXPR uint8_t as_uint8(byte b) noexcept;
REX_CONSTEXPR byte as_byte(uint8_t b) noexcept;

template <typename T>
byte* as_bytes(T* ptr) noexcept;

template <typename T>
const byte* as_bytes(const T* ptr) noexcept;

template <class IntegerType>
REX_CONSTEXPR byte operator<<(byte b, IntegerType shift) noexcept;
template <class IntegerType>
REX_CONSTEXPR byte operator>>(byte b, IntegerType shift) noexcept;
template <class IntegerType>
REX_CONSTEXPR byte& operator<<=(byte& b, IntegerType shift) noexcept;
template <class IntegerType>
REX_CONSTEXPR byte& operator>>=(byte& b, IntegerType shift) noexcept;

REX_CONSTEXPR byte operator~(byte b) noexcept;
REX_CONSTEXPR byte operator|(byte lhs, byte rhs) noexcept;
REX_CONSTEXPR byte operator&(byte lhs, byte rhs) noexcept;
REX_CONSTEXPR byte operator^(byte lhs, byte rhs) noexcept;
REX_CONSTEXPR byte& operator|=(byte& lhs, byte rhs) noexcept;
REX_CONSTEXPR byte& operator&=(byte& lhs, byte rhs) noexcept;
REX_CONSTEXPR byte& operator^=(byte& lhs, byte rhs) noexcept;

REX_CONSTEXPR std::size_t get_byte_bits() noexcept;

template <typename C>
REX_CONSTEXPR void to_hex(byte b, C &high, C &low) noexcept;

template<typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, const byte& b);

} // namespace rex

template <>
struct std::hash<rex::byte>
{
	REX_CONSTEXPR size_t operator()(rex::byte b) const noexcept;
};

#include "byte.inl"
