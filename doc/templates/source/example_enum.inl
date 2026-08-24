// SPDX-License-Identifier: GPL-3.0-only

#include "example_enum.hpp"

#include <unordered_map>

namespace rex 
{

REXLIB_INLINE_CONSTEXPR 
const char* to_string(example_enum ex) noexcept
{
    switch (ex)
    {
    case example_enum::first: return "first";
    case example_enum::second: return "second";
    case example_enum::third: return "third";
    }
}

inline
bool from_string(std::string_view str, example_enum& ex) noexcept
{
    static const 
    std::unordered_map<std::string_view, example_enum> str_2_example_enum = 
    {
        { to_string(example_enum::first), example_enum::first },
        { to_string(example_enum::second), example_enum::second },
        { to_string(example_enum::third), example_enum::third },
    };

    const auto ite = str_2_example_enum.find(str);
    const auto result = ite != str_2_example_enum.cend();
    if (result)
    {
        ex = ite->second;
    }

    return result;
}

template<typename T>
inline
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, example_enum ex)
{
    return os << to_string(ex);
}

} // namespace rex
