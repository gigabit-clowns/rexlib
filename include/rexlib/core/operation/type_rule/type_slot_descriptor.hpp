// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/type_rule/type_transform.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Type erased form of a type_slot.
 *
 * @see type_slot
 */
class type_slot_descriptor
{
public:
	/**
	 * @brief Construct a slot descriptor.
	 *
	 * @param pivot_index Index of the pivot the operand derives from.
	 * @param transform Transform applied to the pivot type.
	 */
	type_slot_descriptor(
		std::size_t pivot_index,
		const type_transform &transform
	) noexcept;

	/**
	 * @brief Get the index of the pivot the operand derives from.
	 *
	 * @return std::size_t The index.
	 */
	std::size_t get_pivot_index() const noexcept;

	/**
	 * @brief Get the transform applied to the pivot type.
	 *
	 * @return const type_transform& The transform.
	 */
	const type_transform& get_transform() const noexcept;

private:
	std::size_t m_pivot_index;
	type_transform m_transform;
};

} // namespace rexlib

#include "type_slot_descriptor.inl"
