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
 * @brief A Philox4x32 block read as two 64-bit values.
 *
 * The first value joins words 0 and 1 of the block, and the second joins
 * words 2 and 3, the lower word giving the low half each time.
 */
using philox_halves_type = std::array<std::uint64_t, 2>;

/**
 * @brief The purpose a Philox4x32 block serves.
 *
 * The domain fills word 3 of the counter, so blocks serving different
 * purposes never share a counter under the same key.
 */
enum class philox_domain : std::uint32_t
{
	/** @brief A block that random draws are taken from. */
	draw = 0,
	/** @brief A block that a key is derived from. */
	key_derivation = 1
};

/**
 * @brief Make the counter of a Philox4x32 block.
 *
 * Words 0 and 1 of the counter are the low and the high half of
 * @p block. Word 2 is @p stream and word 3 is @p domain.
 *
 * @param block The index of the block within its stream.
 * @param stream The index of the stream within its domain.
 * @param domain The purpose of the block.
 * @return philox_counter_type The counter.
 */
philox_counter_type make_philox_counter(
	std::uint64_t block,
	std::uint32_t stream,
	philox_domain domain
) noexcept;

/**
 * @brief Make a Philox4x32 key from a 64-bit key.
 *
 * Word 0 of the result is the low half of @p key, and word 1 is its high
 * half.
 *
 * @param key The 64-bit key.
 * @return philox_key_type The two words of the key.
 */
philox_key_type make_philox_key(std::uint64_t key) noexcept;

/**
 * @brief Read a Philox4x32 block as two 64-bit values.
 *
 * @param block The four words of the block.
 * @return philox_halves_type Words 0 and 1, then words 2 and 3, each pair
 * joined with its lower word as the low half.
 */
philox_halves_type make_philox_halves(
	const philox_counter_type &block
) noexcept;

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
