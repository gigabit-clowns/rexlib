// SPDX-License-Identifier: GPL-3.0-only

#include "image_open_options.hpp"

#include <rexlib/core/platform/enum_helpers.hpp>

namespace rexlib
{
namespace em
{

REXLIB_INLINE_CONSTEXPR const char*
to_string(image_access_hint v) noexcept
{
	switch (v)
	{
	REXLIB_ENUM_TO_STR_CASE(image_access_hint, none)
	REXLIB_ENUM_TO_STR_CASE(image_access_hint, sequential)
	REXLIB_ENUM_TO_STR_CASE(image_access_hint, random)
	default: return "";
	}
}

template<typename T>
inline std::basic_ostream<T>&
operator<<(std::basic_ostream<T>& os, image_access_hint v)
{
	return os << to_string(v);
}

} // namespace em
} // namespace rexlib
