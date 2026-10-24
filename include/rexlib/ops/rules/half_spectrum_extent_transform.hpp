// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{
namespace ops
{

/**
 * @brief Transform mapping the extent of a real signal onto the extent of
 * its half spectrum.
 *
 * An axis of n samples holds n / 2 + 1 coefficients once transformed, the
 * other half being redundant.
 *
 * @see operand_axis_slot
 */
struct half_spectrum_extent_transform
{
	/**
	 * @brief Apply the transform.
	 *
	 * @param extent The number of samples.
	 * @return std::size_t The number of coefficients, extent / 2 + 1.
	 */
	static std::size_t apply(std::size_t extent) noexcept;

	/**
	 * @brief Get a fragment naming the mapping, for diagnostics.
	 *
	 * @return const char* The fragment.
	 */
	static const char* get_relation() noexcept;
};

} // namespace ops
} // namespace rexlib

#include "half_spectrum_extent_transform.inl"
