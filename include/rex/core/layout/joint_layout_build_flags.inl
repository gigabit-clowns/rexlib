// SPDX-License-Identifier: GPL-3.0-only

#include "joint_layout_build_flags.hpp"
#include "../platform/enum_helpers.hpp"

namespace rexlib 
{

REXLIB_INLINE_CONSTEXPR 
const char* to_string(joint_layout_build_flag_bits x) noexcept
{
	switch (x)
	{
	REXLIB_ENUM_TO_STR_CASE(
		joint_layout_build_flag_bits, 
		enable_reordering
	)
	REXLIB_ENUM_TO_STR_CASE(
		joint_layout_build_flag_bits, 
		enable_coalescing
	)
	default: return "";
	}
}

template<typename T>
inline
std::basic_ostream<T>& operator<<(
	std::basic_ostream<T>& os, 
	joint_layout_build_flag_bits x
)
{
	return os << to_string(x);
}

} // namespace rexlib
