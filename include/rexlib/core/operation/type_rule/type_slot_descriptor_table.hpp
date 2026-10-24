// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/type_slot_descriptor.hpp>

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/span.hpp>

namespace rexlib
{

/**
 * @brief Lower a rule's slot list into a table of descriptors.
 *
 * @tparam SlotList type_list of type_slot declarations.
 */
template <typename SlotList>
struct type_slot_descriptor_table;

template <>
struct type_slot_descriptor_table<type_list<>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const type_slot_descriptor> An empty table.
	 */
	static span<const type_slot_descriptor> get() noexcept;
};

template <typename... Slots>
struct type_slot_descriptor_table<type_list<Slots...>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const type_slot_descriptor> One descriptor per slot, in
	 * declaration order.
	 */
	static span<const type_slot_descriptor> get() noexcept;
};

} // namespace rexlib

#include "type_slot_descriptor_table.inl"
