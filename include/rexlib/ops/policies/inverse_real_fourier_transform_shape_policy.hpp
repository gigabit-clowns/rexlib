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
 * @brief Shape policy for the inverse Fourier transform of a real signal.
 *
 * The inverse of @ref real_fourier_transform_shape_policy. Every
 * transformed axis but the last keeps its extent, and the last takes the
 * extent the policy is given. That axis must hold extent / 2 + 1
 * coefficients.
 *
 * @see real_fourier_transform_shape_policy
 */
class REXLIB_API inverse_real_fourier_transform_shape_policy final
	: public operation_shape_policy
{
public:
	/**
	 * @brief Construct a policy transforming along a set of axes.
	 *
	 * @param axes The transformed axes. The last one named is the restored
	 * one.
	 * @param extent The number of samples along the restored axis.
	 *
	 * @throws std::invalid_argument When an axis is repeated, when no
	 * axis is given, or when @p extent is zero.
	 */
	inverse_real_fourier_transform_shape_policy(
		axis_list axes,
		std::size_t extent
	);

	/**
	 * @brief Get the transformed axes, in the order they were named.
	 *
	 * @return span<const std::size_t> The axes.
	 */
	span<const std::size_t> get_axes() const noexcept;

	/**
	 * @brief Get the number of samples along the restored axis.
	 *
	 * @return std::size_t The extent.
	 */
	std::size_t get_extent() const noexcept;

	void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

private:
	REXLIB_STD_MEMBER_INTERFACE
	axis_list m_axes;
	std::size_t m_extent;
};

} // namespace ops
} // namespace rexlib
