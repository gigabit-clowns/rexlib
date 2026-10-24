// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/type_pivot_descriptor.hpp>

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <utility>

namespace rexlib
{

/**
 * @brief Lower a rule's pivot list into a table of descriptors.
 *
 * A pivot supplied by the operation is given the position of its value
 * among those the operation supplies, which follow the order the pivots
 * are declared in.
 *
 * @tparam PivotList type_list of type_pivot declarations.
 */
template <typename PivotList>
struct type_pivot_descriptor_table;

template <>
struct type_pivot_descriptor_table<type_list<>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const type_pivot_descriptor> An empty table.
	 */
	static span<const type_pivot_descriptor> get() noexcept;
};

template <typename... Pivots>
struct type_pivot_descriptor_table<type_list<Pivots...>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const type_pivot_descriptor> One descriptor per pivot,
	 * in declaration order.
	 */
	static span<const type_pivot_descriptor> get() noexcept;

private:
	template <std::size_t... Positions>
	static span<const type_pivot_descriptor> lower(
		std::index_sequence<Positions...>
	) noexcept;
};

} // namespace rexlib

#include "type_pivot_descriptor_table.inl"
