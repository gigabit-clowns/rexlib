// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "axis_list.hpp"

#include <rexlib/core/dispatch/operation_shape_policy.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <ostream>

namespace rexlib
{
namespace ops
{

/**
 * @brief Whether a transformed real signal had an even or odd extent.
 *
 * The one thing an inverse real transform cannot work out for itself. An
 * axis of `m` stored coefficients came from `2*(m - 1)` samples if it was
 * even and `2*m - 1` if it was odd, and the coefficients look the same
 * either way.
 */
enum class signal_parity
{
	even,
	odd
};

REXLIB_API
const char* to_string(signal_parity parity) noexcept;

REXLIB_API
std::ostream& operator<<(std::ostream &os, signal_parity parity);

/**
 * @brief Shape policy for the inverse Fourier transform of a real signal.
 *
 * The inverse of @ref real_fourier_transform_shape_policy. Every
 * transformed axis but the last keeps its extent, and the last is restored
 * from the halved one according to the parity it had.
 *
 * The parity is the whole of what the operation carries. A caller that
 * would rather name the extent it wants converts it, the two being
 * interchangeable once the operand is at hand.
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
	 * @param axes The transformed axes. Sorted on construction. The last
	 * of them is the restored one.
	 * @param parity The parity the restored axis had before it was
	 * transformed.
	 *
	 * @throws std::invalid_argument When an axis is repeated, or when no
	 * axis is given.
	 */
	inverse_real_fourier_transform_shape_policy(
		axis_list axes,
		signal_parity parity
	);

	/**
	 * @brief Get the transformed axes, in ascending order.
	 *
	 * @return span<const std::size_t> The axes.
	 */
	span<const std::size_t> get_axes() const noexcept;

	/**
	 * @brief Get the parity of the restored axis.
	 *
	 * @return signal_parity The parity.
	 */
	signal_parity get_parity() const noexcept;

	void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

private:
	REXLIB_STD_MEMBER_INTERFACE
	axis_list m_axes;
	signal_parity m_parity;
};

/**
 * @brief Get the number of samples a halved axis was made from.
 *
 * @param stored The number of coefficients stored along the axis.
 * @param parity The parity of the axis before it was transformed.
 * @return std::size_t The number of samples.
 */
REXLIB_API
std::size_t compute_real_signal_extent(
	std::size_t stored,
	signal_parity parity
) noexcept;

/**
 * @brief Get the parity an axis of a given extent has.
 *
 * How a caller naming the extent it wants states it as the operation does.
 *
 * @param extent The number of samples along the axis.
 * @return signal_parity The parity.
 */
REXLIB_API
signal_parity get_signal_parity(std::size_t extent) noexcept;

} // namespace ops
} // namespace rexlib
