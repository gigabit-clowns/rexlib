// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/policies/vector_matrix_shape_policy.hpp>

#include "shape_deduction.hpp"

#include <xmipp4/core/layout/broadcast.hpp>

#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace rex
{
namespace ops
{

namespace
{

using shape_type = operation_shape_policy::shape_type;

void require_vector(
	const operation_descriptor &descriptor,
	const shape_type &shape,
	std::size_t index
)
{
	if (shape.empty())
	{
		std::ostringstream oss;
		oss << descriptor << ": input operand "
			<< describe_operand(descriptor, index, false)
			<< " has no axes, so it is a scalar rather than a vector.";
		throw std::invalid_argument(oss.str());
	}
}

void require_matrix(
	const operation_descriptor &descriptor,
	const shape_type &shape,
	std::size_t index
)
{
	if (shape.size() < 2)
	{
		std::ostringstream oss;
		oss << descriptor << ": input operand "
			<< describe_operand(descriptor, index, false)
			<< " does not have rank two or more, so it is not a matrix.";
		throw std::invalid_argument(oss.str());
	}
}

} // anonymous namespace

void vector_matrix_shape_policy::deduce(
	const operation_descriptor &descriptor,
	span<shape_type> canonical_output_shapes,
	span<const shape_type> input_shapes
) const
{
	if (input_shapes.size() != 2)
	{
		std::ostringstream oss;
		oss << descriptor << ": a vector-matrix product takes exactly two "
			<< "input operands.";
		throw std::invalid_argument(oss.str());
	}

	const auto &left = input_shapes[0];
	const auto &right = input_shapes[1];
	require_vector(descriptor, left, 0);
	require_matrix(descriptor, right, 1);

	const auto inner = left.back();
	const auto other_inner = right[right.size() - 2];
	const auto columns = right.back();

	if (inner != other_inner)
	{
		std::ostringstream oss;
		oss << descriptor << ": input operand "
			<< describe_operand(descriptor, 0, false)
			<< " has " << inner << " elements, but input operand "
			<< describe_operand(descriptor, 1, false)
			<< " has " << other_inner << " rows.";
		throw std::invalid_argument(oss.str());
	}

	// Everything before the vector and the matrix is a stack, broadcast as
	// elsewhere.
	shape_type shape(left.begin(), std::prev(left.end(), 1));
	const shape_type right_batch(right.begin(), std::prev(right.end(), 2));
	broadcast_extents_accumulate(shape, make_span(right_batch));

	shape.push_back(columns);

	assign_output_shapes(canonical_output_shapes, std::move(shape));
}

const vector_matrix_shape_policy&
vector_matrix_shape_policy::get() noexcept
{
	static const vector_matrix_shape_policy instance;
	return instance;
}

} // namespace ops
} // namespace rex
