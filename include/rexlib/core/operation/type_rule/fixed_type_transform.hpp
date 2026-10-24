// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>

namespace rexlib
{

/**
 * @brief Transform ignoring the pivot and yielding a fixed type.
 *
 * Models operations whose operand type does not depend on the others, such
 * as the boolean output of a comparison.
 *
 * @tparam Type The produced numerical type.
 *
 * @ingroup type_transform
 */
template <numerical_type Type>
struct fixed_type_transform
{
	using domain_type = any_type_domain;

	/**
	 * @brief Get a fragment naming the mapping, for diagnostics.
	 *
	 * @return const char* The fragment.
	 */
	static const char* get_relation() noexcept;

	/**
	 * @brief Apply the transform to a pivot type.
	 *
	 * @param type The pivot type.
	 * @return numerical_type The operand type.
	 */
	static numerical_type apply(numerical_type type) noexcept;

	/**
	 * @brief Apply the transform to a native pivot type.
	 *
	 * @tparam T The native pivot type.
	 */
	template <typename T>
	struct apply_type
	{
		using type = typename native_type_of<Type>::type;
	};
};

} // namespace rexlib

#include "fixed_type_transform.inl"
