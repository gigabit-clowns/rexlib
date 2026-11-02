// SPDX-License-Identifier: GPL-3.0-only

#include "box_window.hpp"

#include <cmath>

namespace rexlib
{
namespace cpu
{

inline
std::size_t box_window::get_tap_count() const noexcept
{
	return tap_count;
}

template <typename T>
inline
std::ptrdiff_t box_window::place(double coordinate, T *weights) const noexcept
{
	weights[0] = T(1);
	return static_cast<std::ptrdiff_t>(std::round(coordinate));
}

} // namespace cpu
} // namespace rexlib
