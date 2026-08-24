// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/span.hpp>
#include <rex/core/platform/dynamic_shared_object.h>

#include <vector>

namespace rexlib 
{

class strided_layout;

/**
 * @brief Perform a broadcast operation between two sets of extents.
 * 
 * This operation pads the smallest set of extents with ones at the beginning. 
 * Then, both sets are iterated jointly. If both elements are equal, none of
 * them is modified. If unequal but one of them is 1, it is replaced by the
 * other. Otherwise an exception is thrown.
 * 
 * @param extents1 The first set of extents.
 * @param extents2 The second set of extents.
 * @throws std::broadcast_error when one of of the extents cannot be broadcasted.
 */
REXLIB_API
void broadcast_extents(
	std::vector<std::size_t> &extents1, 
	std::vector<std::size_t> &extents2
);  

/**
 * @brief Perform an cumulative extent broadcast step
 * 
 * @param result The extent vector to be updated. After a successful call, the
 * broadcast result between the previous value of result and other is stored.
 * @param other The other set of extents.
 * @throws std::broadcast_error when one of of the extents cannot be broadcasted.
 */
REXLIB_API
void broadcast_extents_accumulate(
	std::vector<std::size_t> &result, 
	span<const std::size_t> other
);

/**
 * @brief Check if two extents can be broadcasted together.
 * 
 * @param extents1 The first set of extents.
 * @param extents2 The second set of extents.
 * @return true If both shapes are compatible. 
 * @return false If both shapes are not compatible. 
 */
REXLIB_API
bool is_broadcast_compatible(
	span<const std::size_t> extents1,
	span<const std::size_t> extents2
) noexcept;

/**
 * @brief Check if a set of extents can be broadcasted into another.
 * 
 * @param from_extents The extents to be broadcasted.
 * @param to_extents The target set of extents.
 * @return true If both shapes are compatible. 
 * @return false If both shapes are not compatible. 
 */
REXLIB_API
bool is_broadcastable_to(
	span<const std::size_t> from_extents,
	span<const std::size_t> to_extents
) noexcept;

} // namespace rexlib
