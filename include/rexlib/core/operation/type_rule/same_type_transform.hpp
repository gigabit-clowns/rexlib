// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>

namespace rexlib
{

/**
 * @defgroup type_transforms Type transforms
 *
 * A transform maps the type of a rule's pivot onto the type of one operand.
 *
 * Every transform states the same mapping twice:
 *
 *   * `apply` evaluates it on runtime numerical_type values. The type rule
 *     interpreter reads this half to deduce and check operand types.
 *   * `apply_type` evaluates it on native C++ types. Backend type
 *     dispatchers read this half to reify the operand element types once
 *     the pivots have been resolved.
 *
 * The two halves must agree: for every native type T admitted by
 * `domain_type`,
 *
 *     numerical_type_of<typename apply_type<T>::type>::value ==
 *         apply(numerical_type_of<T>::value)
 *
 * This is machine checked for every transform by the transform tests, which
 * is what makes it safe to write a typing rule once and have both sides
 * interpret it.
 *
 * A transform also declares `domain_type`, the set of pivot types it is
 * defined on. A rule intersects it with the domain the operation declares,
 * so a rule can never resolve to a type its own transforms cannot map.
 *
 * `get_relation` returns a fragment naming the mapping, used to build
 * diagnostics such as "output operand 'result' must be the real
 * counterpart of input operand 'value'".
 */

/**
 * @brief Transform mapping a pivot type onto itself.
 *
 * Models operations whose operands all share one type, such as `add`.
 *
 * @ingroup type_transforms
 */
struct same_type_transform
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
		using type = T;
	};
};

} // namespace rexlib

#include "same_type_transform.inl"
