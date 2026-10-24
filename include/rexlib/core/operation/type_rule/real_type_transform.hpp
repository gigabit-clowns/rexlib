// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>

#include <complex>

namespace rexlib
{

/**
 * @brief Transform mapping a pivot type onto its real counterpart.
 *
 * Non complex types are left untouched, so this is defined on every type.
 * Models operations that drop the imaginary part, such as `abs` or `real`.
 *
 * @ingroup type_transforms
 */
struct real_type_transform
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

	template <typename T>
	struct apply_type<std::complex<T>>
	{
		using type = T;
	};
};

} // namespace rexlib

#include "real_type_transform.inl"
