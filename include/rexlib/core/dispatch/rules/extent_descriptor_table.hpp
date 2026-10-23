// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "extent_descriptor.hpp"

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/span.hpp>

namespace rexlib
{

/**
 * @brief Lower the extent variables of a shape rule into a table.
 *
 * @tparam ExtentList type_list of operand_extent declarations.
 */
template <typename ExtentList>
struct extent_descriptor_table;

template <>
struct extent_descriptor_table<type_list<>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const extent_descriptor> An empty table.
	 */
	static span<const extent_descriptor> get() noexcept;
};

template <typename... Extents>
struct extent_descriptor_table<type_list<Extents...>>
{
	/**
	 * @brief Get the table.
	 *
	 * @return span<const extent_descriptor> One descriptor per variable, in
	 * declaration order.
	 */
	static span<const extent_descriptor> get() noexcept;
};

} // namespace rexlib

#include "extent_descriptor_table.inl"
