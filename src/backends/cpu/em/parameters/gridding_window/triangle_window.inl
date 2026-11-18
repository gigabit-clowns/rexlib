// SPDX-License-Identifier: GPL-3.0-only

#include "triangle_window.hpp"

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
void triangle_window::place(
	const T *coordinates,
	std::int32_t count,
	std::int32_t *first,
	T *weights,
	std::size_t weight_stride
) const noexcept
{
	auto *above = weights + weight_stride;
	for (std::int32_t i = 0; i < count; ++i)
	{
		const auto c = coordinates[i];
		const auto truncated = static_cast<std::int32_t>(c);
		const auto below =
			truncated - (c < static_cast<T>(truncated) ? 1 : 0);
		const auto fraction = c - static_cast<T>(below);
		first[i] = below;
		weights[i] = T(1) - fraction;
		above[i] = fraction;
	}
}

} // namespace cpu
} // namespace rexlib
