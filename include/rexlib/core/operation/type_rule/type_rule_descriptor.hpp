// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/type_pivot_descriptor.hpp>
#include <rexlib/core/operation/type_rule/type_slot_descriptor.hpp>

#include <rexlib/core/span.hpp>

namespace rexlib
{

/**
 * @brief Type erased form of an operand_type_rule.
 *
 * @see operand_type_rule
 */
class type_rule_descriptor
{
public:
	/**
	 * @brief Construct a type rule descriptor.
	 *
	 * Nothing is owned: every table must outlive the descriptor.
	 *
	 * @param pivots The pivots.
	 * @param output_slots The slot of every output, in signature order.
	 * @param input_slots The slot of every input, in signature order.
	 */
	type_rule_descriptor(
		span<const type_pivot_descriptor> pivots,
		span<const type_slot_descriptor> output_slots,
		span<const type_slot_descriptor> input_slots
	) noexcept;

	/**
	 * @brief Get the pivots.
	 *
	 * @return span<const type_pivot_descriptor> The pivots.
	 */
	span<const type_pivot_descriptor> get_pivots() const noexcept;

	/**
	 * @brief Get the slot of every output.
	 *
	 * @return span<const type_slot_descriptor> One entry per output.
	 */
	span<const type_slot_descriptor> get_output_slots() const noexcept;

	/**
	 * @brief Get the slot of every input.
	 *
	 * @return span<const type_slot_descriptor> One entry per input.
	 */
	span<const type_slot_descriptor> get_input_slots() const noexcept;

private:
	span<const type_pivot_descriptor> m_pivots;
	span<const type_slot_descriptor> m_output_slots;
	span<const type_slot_descriptor> m_input_slots;
};

} // namespace rexlib

#include "type_rule_descriptor.inl"
