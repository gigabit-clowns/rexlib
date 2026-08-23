// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/dispatch/operation_shape_policy.hpp>
#include <rex/core/platform/dynamic_shared_object.h>

#include <cstddef>

namespace rex
{
namespace ops
{

/**
 * @brief Shape policy for a cross product taken along one axis.
 *
 * The operands are broadcast together and the product is taken along one
 * axis, which survives with the extent it had. Every other axis is a batch
 * dimension, so a stack of vectors crosses with another stack, or with a
 * single vector, without any of them being written out.
 *
 * That axis must have extent three. Unlike the extents a matrix
 * multiplication contracts, this one is not a free parameter of the
 * problem: the cross product exists in three dimensions and the check is
 * what keeps a silently wrong shape from reaching a backend.
 */
class REXLIB_API cross_product_shape_policy final
	: public operation_shape_policy
{
public:
	/// The extent the product is defined on.
	static const std::size_t vector_extent = 3;

	/**
	 * @brief Construct a policy taking the product along an axis.
	 *
	 * @param axis The axis the product is taken along.
	 */
	explicit cross_product_shape_policy(std::size_t axis) noexcept;

	/**
	 * @brief Get the axis the product is taken along.
	 *
	 * @return std::size_t The axis.
	 */
	std::size_t get_axis() const noexcept;

	void deduce(
		const operation_descriptor &descriptor,
		span<shape_type> canonical_output_shapes,
		span<const shape_type> input_shapes
	) const override;

private:
	std::size_t m_axis;
};

} // namespace ops
} // namespace rex
