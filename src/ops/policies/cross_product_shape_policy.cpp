// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/policies/cross_product_shape_policy.hpp>

#include "shape_deduction.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

namespace rex
{
namespace ops
{

const std::size_t cross_product_shape_policy::vector_extent;

cross_product_shape_policy::cross_product_shape_policy(
	std::size_t axis
) noexcept
	: m_axis(axis)
{
}

std::size_t cross_product_shape_policy::get_axis() const noexcept
{
	return m_axis;
}

void cross_product_shape_policy::deduce(
	const operation_descriptor &descriptor,
	span<shape_type> canonical_output_shapes,
	span<const shape_type> input_shapes
) const
{
	if (input_shapes.size() != 2)
	{
		std::ostringstream oss;
		oss << descriptor << ": a cross product takes exactly two input "
			<< "operands.";
		throw std::invalid_argument(oss.str());
	}

	// Broadcasting first is what lets a stack of vectors cross with a
	// single one without either being written out.
	auto shape = broadcast_input_shapes(descriptor, input_shapes);

	if (m_axis >= shape.size())
	{
		std::ostringstream oss;
		oss << descriptor << ": the product was requested along axis "
			<< m_axis << ", but the broadcast operands only have "
			<< shape.size() << " of them.";
		throw std::out_of_range(oss.str());
	}

	if (shape[m_axis] != vector_extent)
	{
		std::ostringstream oss;
		oss << descriptor << ": axis " << m_axis << " has extent "
			<< shape[m_axis] << ", but a cross product is defined on "
			<< vector_extent << " components.";
		throw std::invalid_argument(oss.str());
	}

	assign_output_shapes(canonical_output_shapes, std::move(shape));
}

} // namespace ops
} // namespace rex
