// SPDX-License-Identifier: GPL-3.0-only

#include "image_access_flags.hpp"

#include <rexlib/core/platform/enum_helpers.hpp>

namespace rexlib
{
namespace em
{

REXLIB_INLINE_CONSTEXPR const char*
to_string(image_access_flag_bits v) noexcept
{
	switch (v)
	{
	REXLIB_ENUM_TO_STR_CASE(image_access_flag_bits, ordered_offsets)
	REXLIB_ENUM_TO_STR_CASE(image_access_flag_bits, concurrent_read)
	REXLIB_ENUM_TO_STR_CASE(image_access_flag_bits, memory_resident)
	default: return "";
	}
}

template<typename T>
inline std::basic_ostream<T>&
operator<<(std::basic_ostream<T>& os, image_access_flag_bits v)
{
	return os << to_string(v);
}

} // namespace em
} // namespace rexlib
