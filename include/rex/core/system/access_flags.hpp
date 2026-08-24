// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/constexpr.hpp"
#include "../binary/bit.hpp"
#include "../binary/flagset.hpp"

#include <ostream>

namespace rex 
{

enum class access_flag_bits {
	read = bit(0),
	write = bit(1),
};

using access_flags = flagset<access_flag_bits>;

inline REXLIB_CONST_CONSTEXPR access_flags read_only(access_flag_bits::read);
inline REXLIB_CONST_CONSTEXPR access_flags write_only(access_flag_bits::write);
inline REXLIB_CONST_CONSTEXPR access_flags read_write(
	{access_flag_bits::read, access_flag_bits::write}
);

REXLIB_CONSTEXPR 
const char* to_string(access_flag_bits v) noexcept;

template<typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, access_flag_bits v);

} // namespace rex

#include "access_flags.inl"
