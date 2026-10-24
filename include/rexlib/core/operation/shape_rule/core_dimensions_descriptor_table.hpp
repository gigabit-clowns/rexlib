// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/core_dimensions_descriptor.hpp>

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/span.hpp>

namespace rexlib
{

/**
 * @brief Lower the core axes of every operand of one kind into a table.
 *
 * @tparam CoreList type_list of core_dimensions, one per operand.
 */
template <typename CoreList>
struct core_dimensions_descriptor_table;

template <>
struct core_dimensions_descriptor_table<type_list<>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const core_dimensions_descriptor> An empty table.
	 */
	static span<const core_dimensions_descriptor> get() noexcept;
};

template <typename... Cores>
struct core_dimensions_descriptor_table<type_list<Cores...>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const core_dimensions_descriptor> One descriptor per
	 * operand, in signature order.
	 */
	static span<const core_dimensions_descriptor> get() noexcept;
};

} // namespace rexlib

#include "core_dimensions_descriptor_table.inl"
