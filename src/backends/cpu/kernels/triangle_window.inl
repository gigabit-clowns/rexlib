// SPDX-License-Identifier: GPL-3.0-only

#include "triangle_window.hpp"

#include <cmath>

namespace rexlib
{
namespace cpu
{

inline
std::size_t triangle_window::get_tap_count() const noexcept
{
	return tap_count;
}

template <typename T>
inline
std::ptrdiff_t triangle_window::place(
	double coordinate,
	T *weights
) const noexcept
{
	const auto below = std::floor(coordinate);
	const auto fraction = coordinate - below;
	weights[0] = static_cast<T>(1.0 - fraction);
	weights[1] = static_cast<T>(fraction);
	return static_cast<std::ptrdiff_t>(below);
}

} // namespace cpu
} // namespace rexlib
