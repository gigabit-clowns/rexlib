// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "fourier_normalization.hpp"

#include <rex/core/dispatch/parametric_operation.hpp>
#include <rex/core/platform/dynamic_shared_object.h>
#include <rex/ops/ops_component.hpp>
#include <rex/ops/policies/axis_list.hpp>
#include <rex/ops/policies/inverse_real_fourier_transform_shape_policy.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

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
 * The inverse of rfft. The extent of the restored axis is recovered from
 * the parity the operation carries, an axis of m coefficients coming
 * from either 2*(m - 1) or 2*m - 1 samples.
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API irfft_operation final
	: public parametric_operation<irfft_operation, irfft_operation_traits>
{
public:
	/**
	 * @brief Construct an inverse transform of a real signal.
	 *
	 * @param axes The transformed axes. Sorted on construction. The last of
	 * them is the restored one, and the remaining axes are batch dimensions.
	 * @param parity The parity the restored axis had before it was
	 * transformed.
	 * @param normalization Which of the transform pair carries the scaling.
	 * It is measured against the restored real signal, not against the
	 * coefficients the spectrum actually holds.
	 *
	 * @throws std::invalid_argument When an axis is repeated, or when no
	 * axis is given.
	 */
	irfft_operation(
		axis_list axes,
		signal_parity parity,
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
