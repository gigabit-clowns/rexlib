// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "fourier_normalization.hpp"

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/parameters/axis_list.hpp>
#include <rexlib/ops/parameters/axis_selection.hpp>
#include <rexlib/ops/rules/shape_rules.hpp>
#include <rexlib/ops/rules/type_rules.hpp>

namespace rexlib
{
namespace ops
{

REXLIB_DECLARE_PARAMETRIC_OPERATION_TRAITS(
	ifft,
	ops_component,
	REXLIB_OPERANDS("signal"),
	REXLIB_OPERANDS("spectrum"),
	fourier_transform_shape_rule,
	axis_selection,
	unary_homogeneous_rule<complex_type_domain>
);

/**
 * @brief Transform an array back from the frequency domain.
 *
 * The result stays complex. A spectrum that is known to come from a real
 * signal inverts through irfft instead, which both halves the storage
 * and returns a real array.
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API ifft_operation final
	: public parametric_operation<ifft_operation, ifft_operation_traits>
{
public:
	/**
	 * @brief Construct an inverse transform.
	 *
	 * @param axes The transformed axes. The remaining axes are batch
	 * dimensions.
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
REXLIB_END_TEMPLATE_BASE

} // namespace ops
} // namespace rexlib
