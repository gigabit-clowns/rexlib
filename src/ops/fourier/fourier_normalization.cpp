// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/fourier/fourier_normalization.hpp>

#include <cmath>

namespace rexlib
{
namespace ops
{

const char* to_string(fourier_normalization normalization) noexcept
{
	switch (normalization)
	{
	case fourier_normalization::backward: return "backward";
	case fourier_normalization::ortho: return "ortho";
	case fourier_normalization::forward: return "forward";
	default: return "";
	}
}

std::ostream& operator<<(
	std::ostream &os,
	fourier_normalization normalization
)
{
	return os << to_string(normalization);
}

const char* to_string(fourier_direction direction) noexcept
{
	switch (direction)
	{
	case fourier_direction::forward: return "forward";
	case fourier_direction::inverse: return "inverse";
	default: return "";
	}
}

std::ostream& operator<<(std::ostream &os, fourier_direction direction)
{
	return os << to_string(direction);
}

double get_fourier_scale(
	fourier_normalization normalization,
	fourier_direction direction,
	std::size_t sample_count
) noexcept
{
	// A transform of nothing has nothing to scale, and dividing by the count
	// would be dividing by zero.
	if (sample_count == 0)
	{
		return 1.0;
	}

	const auto samples = static_cast<double>(sample_count);
	const auto scaled = 1.0 / samples;

	switch (normalization)
	{
	case fourier_normalization::ortho:
		// Its own mirror image, so the direction does not come into it.
		return 1.0 / std::sqrt(samples);

	case fourier_normalization::forward:
		return direction == fourier_direction::forward ? scaled : 1.0;

	case fourier_normalization::backward:
	default:
		return direction == fourier_direction::forward ? 1.0 : scaled;
	}
}

} // namespace ops
} // namespace rexlib
