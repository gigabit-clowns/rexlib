// SPDX-License-Identifier: GPL-3.0-only

#include "axis_phase.hpp"

#include "pi.hpp"

#include <cmath>

namespace rexlib
{
namespace cpu
{

template <typename T>
inline
void fill_axis_phase(
	std::complex<T> *out,
	std::size_t count,
	std::size_t extent,
	double shift
) noexcept
{
	const auto scale = 2.0 * pi * shift / static_cast<double>(extent);
	const auto half = extent / 2;
	for (std::size_t j = 0; j < count; ++j)
	{
		const auto frequency = j <= half
			? static_cast<double>(j)
			: static_cast<double>(j) - static_cast<double>(extent);
		const auto angle = scale * frequency;
		out[j] = std::complex<T>(
			static_cast<T>(std::cos(angle)),
			static_cast<T>(std::sin(angle))
		);
	}
}

} // namespace cpu
} // namespace rexlib
