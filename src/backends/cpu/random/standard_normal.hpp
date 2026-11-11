// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <array>
#include <cstdint>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Map 64 random bits to a uniform value in (0, 1].
 *
 * The top 53 bits are kept, so the value has the full precision of a
 * double. It is never 0, which keeps its logarithm finite.
 *
 * @param bits The random bits.
 * @return double A value in (0, 1], a multiple of 2^-53.
 */
double uniform_open_closed(std::uint64_t bits) noexcept;

/**
 * @brief Draw a pair of independent standard normal values.
 *
 * The pair is the Box-Muller transform of a Philox4x32-10 block keyed by
 * @p key. The counter of the block is @p block in stream 0 of the draw
 * domain. The first half of the block gives the uniform value of the
 * radius, and the second half gives the uniform value of the angle. The
 * first value of the pair is the cosine branch and the second value is the
 * sine branch.
 *
 * @param key The key.
 * @param block The index of the pair.
 * @return std::array<double, 2> Two values of the standard normal
 * distribution.
 */
std::array<double, 2> standard_normal_pair(
	std::uint64_t key,
	std::uint64_t block
) noexcept;

/**
 * @brief Draw the standard normal value at a counter.
 *
 * Counters 2k and 2k + 1 share the pair at block k, and take its first and
 * its second value.
 *
 * @param key The key.
 * @param counter The index of the value.
 * @return double A value of the standard normal distribution.
 */
double standard_normal(std::uint64_t key, std::uint64_t counter) noexcept;

} // namespace cpu
} // namespace rexlib

#include "standard_normal.inl"
