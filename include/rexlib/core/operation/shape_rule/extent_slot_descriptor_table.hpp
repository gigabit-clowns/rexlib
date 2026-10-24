// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/extent_slot_descriptor.hpp>

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/span.hpp>

namespace rexlib
{

/**
 * @brief Lower the slots of one core into a table of descriptors.
 *
 * @tparam SlotList type_list of extent_slot declarations.
 */
template <typename SlotList>
struct extent_slot_descriptor_table;

template <>
struct extent_slot_descriptor_table<type_list<>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const extent_slot_descriptor> An empty table.
	 */
	static span<const extent_slot_descriptor> get() noexcept;
};

template <typename... Slots>
struct extent_slot_descriptor_table<type_list<Slots...>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const extent_slot_descriptor> One descriptor per slot,
	 * in declaration order.
	 */
	static span<const extent_slot_descriptor> get() noexcept;
};

} // namespace rexlib

#include "extent_slot_descriptor_table.inl"
