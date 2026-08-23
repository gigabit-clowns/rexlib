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

REX_DECLARE_OPERATION_TRAITS(
	ifft,
	ops_component,
	REX_OPERANDS("signal"),
	REX_OPERANDS("spectrum"),
	axiswise_operation_shape_policy,
	unary_homogeneous_rule<complex_type_domain>
);

/**
 * @brief Transform an array back from the frequency domain.
 *
 * The result stays complex. A spectrum that is known to come from a real
 * signal inverts through irfft instead, which both halves the storage
 * and returns a real array.
 */
REX_BEGIN_TEMPLATE_BASE
class REXLIB_API ifft_operation final
	: public parametric_operation<ifft_operation, ifft_operation_traits>
{
public:
	/**
	 * @brief Construct an inverse transform.
	 *
	 * @param axes The transformed axes. Sorted on construction. The
	 * remaining axes are batch dimensions.
	 * @param normalization Which of the transform pair carries the scaling.
	 * An inverse transform divides by the number of samples under the
	 * default one, which is what undoes the forward transform.
	 *
	 * @throws std::invalid_argument When an axis is repeated.
	 */
	explicit ifft_operation(
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
REX_END_TEMPLATE_BASE

} // namespace ops
} // namespace xmipp4
