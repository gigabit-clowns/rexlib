// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "example_flags.hpp"

#include <unordered_map>

namespace rex
{

REX_INLINE_CONSTEXPR 
const char* to_string(example_flag_bits ex) noexcept
{
    switch (ex)
    {
    case example_flag_bits::first: return "first";
    case example_flag_bits::second: return "second";
    case example_flag_bits::third: return "third";
    }
}

inline
bool from_string(std::string_view str, example_flag_bits& ex) noexcept
{
    static const
    std::unordered_map<std::string_view, example_flag_bits> str_2_example_flag_bits = 
    {
        { to_string(example_flag_bits::first), example_flag_bits::first },
        { to_string(example_flag_bits::second), example_flag_bits::second },
        { to_string(example_flag_bits::third), example_flag_bits::third },
    };

    const auto ite = str_2_example_flag_bits.find(str);
    const bool result = ite != str_2_example_flag_bits.end();
    if(result)
        ex = ite->second;
    return result;
}

template<typename T>
inline
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, enumeration ex)
{
    return os << to_string(ex);
}

} // namespace rex
