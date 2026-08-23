// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "platform/constexpr.hpp"
#include "binary/flagset.hpp"

#include <string_view>
#include <ostream>

namespace xmipp4
{

enum class example_flag_bits
{
    first = binary::bit(0),
    second = binary::bit(1),
    third = binary::bit(2),
};

using example_flags = binary::flagset<example_flag_bits>;

REX_CONSTEXPR const char* to_string(example_flag_bits ex) noexcept;
bool from_string(std::string_view str, example_flag_bits& ex) noexcept;

template<typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, enumeration ex);

template<typename B>
struct flag_traits;

template<>
struct flag_traits<hardware::example_flag_bits>
{
    using flagset_type = hardware::example_flags;

    static inline REX_CONST_CONSTEXPR flagset_type all = 
    {
        example_flag_bits::first,
        example_flag_bits::second,
        example_flag_bits::third,
    };
};

} // namespace xmipp4

#include "example_flags.inl"
