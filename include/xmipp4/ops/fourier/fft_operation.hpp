// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "fourier_normalization.hpp"

#include <xmipp4/core/dispatch/parametric_operation.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/axis_list.hpp>
#include <xmipp4/ops/policies/axiswise_operation_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace xmipp4
{
namespace ops
{

XMIPP4_DECLARE_OPERATION_TRAITS(
	fft,
	ops_component,
	XMIPP4_OPERANDS("spectrum"),
	XMIPP4_OPERANDS("signal"),
	axiswise_operation_shape_policy,
	unary_complex_of_rule<inexact_type_domain>
);

/**
 * @brief Transform an array to the frequency domain.
 *
 * The transform is n dimensional: it acts along a set of axes and leaves
 * the rest as batch dimensions, so one operation serves the one, two and
 * three dimensional cases alike.
 *
 * A real operand is admitted and produces the complex spectrum of the
 * same precision. The full spectrum is stored, redundant halves and all;
 * use rfft to store only what a real signal needs.
 */
XMIPP4_BEGIN_TEMPLATE_BASE
class REXLIB_API fft_operation final
	: public parametric_operation<fft_operation, fft_operation_traits>
{
public:
	/**
	 * @brief Construct a forward transform.
	 *
	 * @param axes The transformed axes. Sorted on construction. The
	 * remaining axes are batch dimensions.
	 * @param normalization Which of the transform pair carries the scaling.
	 * A forward transform is left as it is computed under the default one.
	 *
	 * @throws std::invalid_argument When an axis is repeated.
	 */
	explicit fft_operation(
		axis_list axes,
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
XMIPP4_END_TEMPLATE_BASE

} // namespace ops
} // namespace xmipp4
