// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <array>
#include <cstdint>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The counter of a Philox4x32 block: four 32-bit words.
 */
using philox_counter_type = std::array<std::uint32_t, 4>;

/**
 * @brief The key of a Philox4x32 generator: two 32-bit words.
 */
using philox_key_type = std::array<std::uint32_t, 2>;

/**
 * @brief Compute the Philox4x32-10 block of a counter under a key.
 *
 * Philox is the counter-based generator of Salmon et al. (2011), as the
 * Random123 library defines it. A block depends on its counter and its key
 * alone, so any block can be computed on its own, in any order and on any
 * thread.
 *
 * @param counter The counter.
 * @param key The key.
 * @return philox_counter_type Four random 32-bit words.
 */
philox_counter_type philox4x32_10(
	philox_counter_type counter,
	philox_key_type key
) noexcept;

} // namespace cpu
} // namespace rexlib

#include "philox.inl"
