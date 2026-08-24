// SPDX-License-Identifier: GPL-3.0-only

#include "access_flags.hpp"
#include "../platform/enum_helpers.hpp"

namespace rex 
{

REXLIB_INLINE_CONSTEXPR const char* 
to_string(access_flag_bits v) noexcept
{
	switch (v)
	{
	REXLIB_ENUM_TO_STR_CASE(access_flag_bits, read)
	REXLIB_ENUM_TO_STR_CASE(access_flag_bits, write)
	default: return "";
	}
}

template<typename T>
inline std::basic_ostream<T>& 
operator<<(std::basic_ostream<T>& os, access_flag_bits v)
{
	return os << to_string(v);
}

} // namespace rex
