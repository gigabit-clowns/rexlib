// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "fourier_normalization.hpp"

#include <rexlib/core/dispatch/parametric_operation.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/policies/axis_list.hpp>
#include <rexlib/ops/policies/inverse_real_fourier_transform_shape_policy.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

#include <cstddef>

namespace rexlib
{
namespace ops
{

REXLIB_DECLARE_OPERATION_TRAITS(
	irfft,
	ops_component,
	REXLIB_OPERANDS("signal"),
	REXLIB_OPERANDS("spectrum"),
	inverse_real_fourier_transform_shape_policy,
	unary_real_of_rule<complex_type_domain>
);

/**
 * @brief Transform a half spectrum back into a real array.
 *
 * The inverse of rfft. An axis of m coefficients comes from either
 * 2*(m - 1) or 2*m - 1 samples, so the operation carries the extent of the
 * restored axis.
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API irfft_operation final
	: public parametric_operation<irfft_operation, irfft_operation_traits>
{
public:
	/**
	 * @brief Construct an inverse transform of a real signal.
	 *
	 * @param axes The transformed axes. The last one named is the restored
	 * one, and the remaining axes are batch dimensions.
	 * @param extent The number of samples along the restored axis. The
	 * spectrum must hold extent / 2 + 1 coefficients along it.
	 * @param normalization Which of the transform pair carries the scaling.
	 * It is measured against the restored real signal, not against the
	 * coefficients the spectrum actually holds.
	 *
	 * @throws std::invalid_argument When an axis is repeated, when no axis
	 * is given, or when @p extent is zero.
	 */
	irfft_operation(
		axis_list axes,
		std::size_t extent,
		fourier_normalization normalization = fourier_normalization::backward
	);

	/**
	 * @brief Get the scaling convention.
	 *
	 * @return fourier_normalization The convention.
	 */
	fourier_normalization get_normalization() const noexcept;

private:
	fourier_normalization m_normalization;
};
REXLIB_END_TEMPLATE_BASE

} // namespace ops
} // namespace rexlib
