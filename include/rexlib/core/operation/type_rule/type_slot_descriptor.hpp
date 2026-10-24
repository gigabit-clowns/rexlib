// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain.hpp>

#include <cstddef>
#include <functional>

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
	 * @brief Value level half of a transform.
	 */
	using transform_function = numerical_type (*)(numerical_type);

	/**
	 * @brief Construct a slot descriptor.
	 *
	 * @param pivot_index Index of the pivot the operand derives from.
	 * @param apply Value level half of the transform. Must not be null.
	 * @param transform_domain Set of pivot types the transform is defined
	 * on. It must outlive the descriptor.
	 * @param relation Fragment naming the mapping, for diagnostics. Must
	 * not be null and must outlive the descriptor.
	 */
	type_slot_descriptor(
		std::size_t pivot_index,
		transform_function apply,
		const numerical_type_domain &transform_domain,
		const char *relation
	) noexcept;

	/**
	 * @brief Get the index of the pivot the operand derives from.
	 *
	 * @return std::size_t The index.
	 */
	std::size_t get_pivot_index() const noexcept;

	/**
	 * @brief Apply the transform to a pivot value.
	 *
	 * @param type The pivot value. Must belong to the transform's domain.
	 * @return numerical_type The operand type.
	 */
	numerical_type apply(numerical_type type) const noexcept;

	/**
	 * @brief Get the set of pivot types the transform is defined on.
	 *
	 * @return const numerical_type_domain& The domain.
	 */
	const numerical_type_domain& get_transform_domain() const noexcept;

	/**
	 * @brief Get the fragment naming the mapping.
	 *
	 * @return const char* The fragment.
	 */
	const char* get_relation() const noexcept;

private:
	std::size_t m_pivot_index;
	transform_function m_apply;
	std::reference_wrapper<const numerical_type_domain> m_transform_domain;
	const char *m_relation;
};

} // namespace rexlib

#include "type_slot_descriptor.inl"
