// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "platform/constexpr.hpp"

#include <string_view>
#include <ostream>

namespace rex 
{

/**
 * @brief Describe the purpose of the example_enum
 * 
 */
enum class example_enum
{
    first,
    second,
    third,
};

REX_CONSTEXPR const char* to_string(example_enum ex) noexcept;
bool from_string(std::string_view str, example_enum& ex) noexcept;

template<typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, example_enum ex);

} // namespace rex

#include "example_enum.inl"
