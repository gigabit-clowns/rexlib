// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdint>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Derive a key from a key and a value.
 *
 * The derived key is the first half of the Philox4x32-10 block keyed by
 * @p key whose counter is @p data in stream 0 of the key derivation domain.
 * Distinct values give independent keys, and no derived key shares a block
 * with the draws of its parent key.
 *
 * @param key The parent key.
 * @param data The value folded into the parent key.
 * @return std::uint64_t The derived key.
 */
std::uint64_t derive_key(std::uint64_t key, std::uint64_t data) noexcept;

} // namespace cpu
} // namespace rexlib

#include "random_key.inl"
