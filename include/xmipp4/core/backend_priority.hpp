// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "platform/constexpr.hpp"
#include "platform/attributes.hpp"

#include <limits>
#include <ostream>

namespace xmipp4 
{

/**
 * @brief Enumeration defining the preference over the selection of a
 * particular backend.
 * 
 */
enum class backend_priority
{
	unsupported = std::numeric_limits<int>::min(), ///< Backend isn't supported.
	fallback = -1024, ///< Used for dummy implementations.
	normal = 0, ///< Normal priority. Use by default.
	optimal = 1024, ///< Backend specially optimized for the use case.
};

REX_CONSTEXPR 
bool operator<(backend_priority lhs, backend_priority rhs) noexcept;
REX_CONSTEXPR 
bool operator<=(backend_priority lhs, backend_priority rhs) noexcept;
REX_CONSTEXPR 
bool operator>(backend_priority lhs, backend_priority rhs) noexcept;
REX_CONSTEXPR 
bool operator>=(backend_priority lhs, backend_priority rhs) noexcept;

REX_CONSTEXPR const char* to_string(backend_priority priority) noexcept;

template<typename T>
std::basic_ostream<T>& operator<<(
	std::basic_ostream<T>& os, 
	backend_priority priority
);

} // namespace xmipp4

#include "backend_priority.inl"
