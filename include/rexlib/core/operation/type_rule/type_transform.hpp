// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain.hpp>

#include <functional>

namespace rexlib
{

/**
 * @brief Type erased form of a type transform.
 *
 * Holds the value level half of a transform, the set of pivot types it is
 * defined on, and the fragment naming it in diagnostics.
 *
 * @see type_transforms
 */
class type_transform
{
public:
	/**
	 * @brief Value level half of a transform.
	 */
	using apply_function_type = numerical_type (*)(numerical_type);

	/**
	 * @brief Construct a type transform.
	 *
	 * @param apply Value level half of the transform. Must not be null.
	 * @param domain Set of pivot types the transform is defined on. It must
	 * outlive the transform, which the domain tags guarantee by holding it
	 * in static storage.
	 * @param relation Fragment naming the mapping, for diagnostics. Must
	 * not be null and must outlive the transform.
	 */
	type_transform(
		apply_function_type apply,
		const numerical_type_domain &domain,
		const char *relation
	) noexcept;

	/**
	 * @brief Apply the transform to a pivot type.
	 *
	 * @param type The pivot type. Must belong to the domain.
	 * @return numerical_type The operand type.
	 */
	numerical_type apply(numerical_type type) const noexcept;

	/**
	 * @brief Get the set of pivot types the transform is defined on.
	 *
	 * @return const numerical_type_domain& The domain.
	 */
	const numerical_type_domain& get_domain() const noexcept;

	/**
	 * @brief Get the fragment naming the mapping.
	 *
	 * @return const char* The fragment.
	 */
	const char* get_relation() const noexcept;

private:
	apply_function_type m_apply;
	std::reference_wrapper<const numerical_type_domain> m_domain;
	const char *m_relation;
};

/**
 * @brief Erase a type transform tag.
 *
 * @tparam Transform A type transform, such as same_type_transform.
 * @return type_transform The erased transform.
 */
template <typename Transform>
type_transform make_type_transform() noexcept;

} // namespace rexlib

#include "type_transform.inl"
