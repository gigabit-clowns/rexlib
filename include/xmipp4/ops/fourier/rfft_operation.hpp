// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "fourier_normalization.hpp"

#include <xmipp4/core/dispatch/parametric_operation.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>
#include <xmipp4/ops/ops_component.hpp>
#include <xmipp4/ops/policies/axis_list.hpp>
#include <xmipp4/ops/policies/real_fourier_transform_shape_policy.hpp>
#include <xmipp4/ops/rules/operand_type_rules.hpp>

namespace xmipp4
{
namespace ops
{

REX_DECLARE_OPERATION_TRAITS(
	rfft,
	ops_component,
	REX_OPERANDS("spectrum"),
	REX_OPERANDS("signal"),
	real_fourier_transform_shape_policy,
	unary_complex_of_rule<floating_point_type_domain>
);

/**
 * @brief Transform a real array to the frequency domain.
 *
 * The spectrum of a real signal is conjugate symmetric, so only the half
 * that is not redundant is stored: the last transformed axis holds
 * n / 2 + 1 coefficients, and every other keeps its extent.
 *
 * Complex operands are not admitted. Their spectra have no symmetry to
 * exploit, and storing half of one would lose information.
 */
REX_BEGIN_TEMPLATE_BASE
class REXLIB_API rfft_operation final
	: public parametric_operation<rfft_operation, rfft_operation_traits>
{
public:
	/**
	 * @brief Construct a forward transform of a real signal.
	 *
	 * @param axes The transformed axes. Sorted on construction. The last of
	 * them is the halved one, and the remaining axes are batch dimensions.
	 * @param normalization Which of the transform pair carries the scaling.
	 * It is measured against the real signal, not against the coefficients
	 * actually stored.
	 *
	 * @throws std::invalid_argument When an axis is repeated, or when no
	 * axis is given.
	 */
	explicit rfft_operation(
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
