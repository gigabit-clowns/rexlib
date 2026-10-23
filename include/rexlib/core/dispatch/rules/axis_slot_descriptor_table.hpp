// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "axis_slot_descriptor.hpp"

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/span.hpp>

namespace rexlib
{

/**
 * @brief Lower the slots of the core axes of an operand into a table.
 *
 * @tparam SlotList type_list of operand_axis_slot declarations.
 */
template <typename SlotList>
struct axis_slot_descriptor_table;

template <>
struct axis_slot_descriptor_table<type_list<>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const axis_slot_descriptor> An empty table.
	 */
	static span<const axis_slot_descriptor> get() noexcept;
};

template <typename... Slots>
struct axis_slot_descriptor_table<type_list<Slots...>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const axis_slot_descriptor> One descriptor per slot, in
	 * declaration order.
	 */
	static span<const axis_slot_descriptor> get() noexcept;
};

} // namespace rexlib

#include "axis_slot_descriptor_table.inl"
