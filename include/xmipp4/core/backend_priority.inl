// SPDX-License-Identifier: GPL-3.0-only

#include "backend_priority.hpp"
#include "platform/enum_helpers.hpp"

namespace xmipp4 
{

REX_INLINE_CONSTEXPR 
bool operator<(backend_priority lhs, backend_priority rhs) noexcept
{
	return static_cast<std::underlying_type<backend_priority>::type>(lhs) <
		static_cast<std::underlying_type<backend_priority>::type>(rhs) ;  
}

REX_INLINE_CONSTEXPR 
bool operator<=(backend_priority lhs, backend_priority rhs) noexcept
{
	return static_cast<std::underlying_type<backend_priority>::type>(lhs) <=
		static_cast<std::underlying_type<backend_priority>::type>(rhs) ;  
}

REX_INLINE_CONSTEXPR 
bool operator>(backend_priority lhs, backend_priority rhs) noexcept
{
	return rhs < lhs;
}

REX_INLINE_CONSTEXPR 
bool operator>=(backend_priority lhs, backend_priority rhs) noexcept
{
	return rhs <= lhs;
}

REX_INLINE_CONSTEXPR 
const char* to_string(backend_priority priority) noexcept
{
	switch (priority)
	{
	REX_ENUM_TO_STR_CASE(backend_priority, unsupported)
	REX_ENUM_TO_STR_CASE(backend_priority, fallback)
	REX_ENUM_TO_STR_CASE(backend_priority, normal)
	REX_ENUM_TO_STR_CASE(backend_priority, optimal)
	default: return "";
	}
}

template<typename T>
inline
std::basic_ostream<T>& operator<<(
	std::basic_ostream<T>& os, 
	backend_priority priority
)
{
	return os << to_string(priority);
}

} // namespace xmipp4
