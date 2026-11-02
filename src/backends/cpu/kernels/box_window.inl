// SPDX-License-Identifier: GPL-3.0-only

#include "box_window.hpp"

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
void box_window::place(
	const T *coordinates,
	std::int32_t count,
	std::int32_t *first,
	T *weights,
	std::size_t
) const noexcept
{
	// Truncating c + 1/2 or c - 1/2 rounds halves away from 0.
	for (std::int32_t i = 0; i < count; ++i)
	{
		const auto c = coordinates[i];
		const auto half = c < T(0) ? T(-0.5) : T(0.5);
		first[i] = static_cast<std::int32_t>(c + half);
		weights[i] = T(1);
	}
}

} // namespace cpu
} // namespace rexlib
