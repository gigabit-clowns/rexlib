// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/any_extent_domain.hpp>
#include <rexlib/core/operation/shape_rule/operand_axis_slot.hpp>

#include <cstddef>

namespace rexlib
{
namespace test
{

/**
 * @brief Extent transform doubling an extent.
 */
struct doubled_extent_transform
{
	static std::size_t apply(std::size_t extent) noexcept
	{
		return 2 * extent;
	}

	static const char* get_relation() noexcept
	{
		return "twice the extent of";
	}
};

/**
 * @brief Axis holding twice an extent variable.
 *
 * @tparam E Index of the extent variable.
 */
template <std::size_t E>
using axis_doubled_of = operand_axis_slot<
	axis_slot_kind::extent,
	E,
	doubled_extent_transform,
	any_extent_domain
>;

} // namespace test
} // namespace rexlib
