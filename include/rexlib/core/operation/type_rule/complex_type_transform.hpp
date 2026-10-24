// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>

#include <complex>

namespace rexlib
{

/**
 * @brief Transform mapping a pivot type onto its complex counterpart.
 *
 * Only floating point and complex types have one, hence the restricted
 * domain. Models operations that introduce an imaginary part.
 *
 * @ingroup type_transforms
 */
struct complex_type_transform
{
	using domain_type = inexact_type_domain;

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
		using type = std::complex<T>;
	};

	template <typename T>
	struct apply_type<std::complex<T>>
	{
		using type = std::complex<T>;
	};
};

} // namespace rexlib

#include "complex_type_transform.inl"
