// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/dispatch/operand_signature.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>

#include <array>
#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Copy the data type of every operand signature into an array.
 *
 * The array carries the operand count in its type, which is what the type
 * dispatchers deduce their arity from.
 *
 * @tparam Count The operand count.
 * @param types Destination, sized to the operand count.
 * @param signatures The operand signatures.
 */
template <std::size_t Count>
void extract_data_types(
	std::array<numerical_type, Count> &types,
	span<const operand_signature> signatures
) noexcept;

} // namespace cpu
} // namespace rexlib

#include "operand_data_types.inl"
