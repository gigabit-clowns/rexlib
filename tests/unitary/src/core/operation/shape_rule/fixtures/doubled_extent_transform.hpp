// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/extent_slot.hpp>

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
using axis_doubled_of = extent_slot<E, doubled_extent_transform>;

} // namespace test
} // namespace rexlib
