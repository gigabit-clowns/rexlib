// SPDX-License-Identifier: GPL-3.0-only

#include "byte.hpp"

#include <array>
#include <climits>

namespace xmipp4
{

REX_INLINE_CONSTEXPR uint8_t 
as_uint8(byte b) noexcept
{
	return static_cast<uint8_t>(b);
}

REX_INLINE_CONSTEXPR byte 
as_byte(uint8_t b) noexcept
{
	return static_cast<byte>(b);
}

template <typename T>
inline byte* 
as_bytes(T* ptr) noexcept
{
	return reinterpret_cast<byte*>(ptr);
}

template <typename T>
inline const byte* 
as_bytes(const T* ptr) noexcept
{
	return reinterpret_cast<const byte*>(ptr);
}

template <class IntegerType>
REX_INLINE_CONSTEXPR byte 
operator<<(byte b, IntegerType shift) noexcept
{
	return as_byte(as_uint8(b) << shift);
}

template <class IntegerType>
REX_INLINE_CONSTEXPR byte 
operator>>(byte b, IntegerType shift) noexcept
{
	return as_byte(as_uint8(b) >> shift);
}

template <class IntegerType>
REX_INLINE_CONSTEXPR byte& 
operator<<=(byte& b, IntegerType shift) noexcept
{
	b = b << shift;
	return b;
}

template <class IntegerType>
REX_INLINE_CONSTEXPR byte& 
operator>>=(byte& b, IntegerType shift) noexcept
{
	b = b >> shift;
	return b;
}

REX_INLINE_CONSTEXPR byte 
operator~(byte b) noexcept
{
	return as_byte(~as_uint8(b));
}

REX_INLINE_CONSTEXPR byte 
operator|(byte lhs, byte rhs) noexcept
{
	return as_byte(as_uint8(lhs) | as_uint8(rhs));
}

REX_INLINE_CONSTEXPR byte 
operator&(byte lhs, byte rhs) noexcept
{
	return as_byte(as_uint8(lhs) & as_uint8(rhs));
}

REX_INLINE_CONSTEXPR byte 
operator^(byte lhs, byte rhs) noexcept
{
	return as_byte(as_uint8(lhs) ^ as_uint8(rhs));
}

REX_INLINE_CONSTEXPR byte& 
operator|=(byte& lhs, byte rhs) noexcept
{
	lhs = lhs | rhs;
	return lhs;
}

REX_INLINE_CONSTEXPR byte& 
operator&=(byte& lhs, byte rhs) noexcept
{
	lhs = lhs & rhs;
	return lhs;
}

REX_INLINE_CONSTEXPR byte& 
operator^=(byte& lhs, byte rhs) noexcept
{
	lhs = lhs ^ rhs;
	return lhs;
}

REX_INLINE_CONSTEXPR std::size_t 
get_byte_bits() noexcept
{
	#if defined(CHAR_BIT)
		return CHAR_BIT;
	#else
		#pragma message("Could not determine byte size. Defaulting to 8")
		return 8UL;
	#endif
}

template <typename C>
REX_INLINE_CONSTEXPR 
void to_hex(byte b, C &high, C &low) noexcept
{
	REX_CONST_CONSTEXPR std::array<C, 16> hex_characters = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
		'A', 'B', 'C', 'D', 'E', 'F'
	};

	const auto value = as_uint8(b);
	high = hex_characters[value >> 4];
	low = hex_characters[value & 0x0F];
}

template<typename T>
inline
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, const byte& b)
{
	T high;
	T low;
	to_hex(b, high, low);
	return os << high << low;
}

} // namespace xmipp4

REX_INLINE_CONSTEXPR size_t 
std::hash<xmipp4::byte>::operator()(xmipp4::byte b) const noexcept
{
	return xmipp4::as_uint8(b);
}
