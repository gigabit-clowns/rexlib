// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/extent_transform.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Type erased form of an extent_slot.
 *
 * @see extent_slot
 */
class extent_slot_descriptor
{
public:
	/**
	 * @brief Construct a slot descriptor.
	 *
	 * @param pivot_index Index of the extent variable the axis derives
	 * from.
	 * @param transform Transform applied to the extent of the variable.
	 */
	extent_slot_descriptor(
		std::size_t pivot_index,
		const extent_transform &transform
	) noexcept;

	/**
	 * @brief Get the index of the extent variable the axis derives from.
	 *
	 * @return std::size_t The index.
	 */
	std::size_t get_pivot_index() const noexcept;

	/**
	 * @brief Get the transform applied to the extent of the variable.
	 *
	 * @return const extent_transform& The transform.
	 */
	const extent_transform& get_transform() const noexcept;

private:
	std::size_t m_pivot_index;
	extent_transform m_transform;
};

} // namespace rexlib

#include "extent_slot_descriptor.inl"
