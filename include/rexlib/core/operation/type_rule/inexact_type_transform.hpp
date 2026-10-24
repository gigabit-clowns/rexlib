// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>

#include <complex>

namespace rexlib
{

/**
 * @brief Transform mapping a pivot type onto its inexact counterpart.
 *
 * Booleans and integers map onto float64, while floating point and complex
 * types are left untouched. Models operations whose result is not
 * representable in the operand's own type, such as a mean.
 *
 * This is the one transform that decides a type rather than deriving it: an
 * integral operand yields float64, and no exact type would do.
 *
 * @ingroup type_transforms
 */
struct inexact_type_transform
{
	using domain_type = domain_union<
		boolean_type_domain,
		arithmetic_type_domain
	>;

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
		using type = float64_t;
	};

	template <typename T>
	struct apply_type<std::complex<T>>
	{
		using type = std::complex<T>;
	};
};

template <>
struct inexact_type_transform::apply_type<float16_t>
{
	using type = float16_t;
};

template <>
struct inexact_type_transform::apply_type<float32_t>
{
	using type = float32_t;
};

template <>
struct inexact_type_transform::apply_type<float64_t>
{
	using type = float64_t;
};

} // namespace rexlib

#include "inexact_type_transform.inl"
