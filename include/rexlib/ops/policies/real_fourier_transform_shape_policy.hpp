// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "axis_list.hpp"

#include <rexlib/core/dispatch/operation_shape_policy.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{
namespace ops
{

/**
 * @brief Shape policy for a Fourier transform of a real operand.
 *
 * The transform of a real signal is conjugate symmetric, so almost half of
 * its coefficients are redundant and are not stored. **Only the last
 * transformed axis is halved**, to `n / 2 + 1`: the symmetry relates a
 * coefficient to the one at the negated index along *every* transformed
 * axis at once, so it can only be exploited once. Every other transformed
 * axis keeps its extent.
 *
 * Nothing else is a parameter. Transforming an axis at a length other than
 * its extent is a matter of padding or cropping the operand, which is work
 * of its own and belongs to whoever asks for it.
 *
 * @see inverse_real_fourier_transform_shape_policy
 */
class REXLIB_API real_fourier_transform_shape_policy final
	: public operation_shape_policy
{
public:
	/**
	 * @brief Construct a policy transforming along a set of axes.
	 *
	 * @param axes The transformed axes. Sorted on construction. The last
	 * of them is the halved one.
	 *
	 * @throws std::invalid_argument When an axis is repeated, or when no
	 * axis is given.
	 */
	explicit real_fourier_transform_shape_policy(axis_list axes);

	/**
	 * @brief Get the transformed axes, in ascending order.
	 *
	 * @return span<const std::size_t> The axes.
	 */
	span<const std::size_t> get_axes() const noexcept;

	void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

private:
	REXLIB_STD_MEMBER_INTERFACE
	axis_list m_axes;
};

} // namespace ops
} // namespace rexlib
