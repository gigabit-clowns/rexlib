// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/policies/matrix_multiply_shape_policy.hpp>

#include "shape_deduction.hpp"

#include <rex/core/layout/broadcast.hpp>

#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace ops
{

namespace
{

using shape_type = operation_shape_policy::shape_type;

void require_non_empty(
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
			<< " has no axes, so it is a scalar rather than something to "
			<< "multiply as a matrix.";
		throw std::invalid_argument(oss.str());
	}
}

} // anonymous namespace

void matrix_multiply_shape_policy::deduce(
	const operation_descriptor &descriptor,
	span<shape_type> canonical_output_shapes,
	span<const shape_type> input_shapes
) const
{
	if (input_shapes.size() != 2)
	{
		std::ostringstream oss;
		oss << descriptor << ": a matrix multiplication takes exactly two "
			<< "input operands.";
		throw std::invalid_argument(oss.str());
	}

	const auto &left = input_shapes[0];
	const auto &right = input_shapes[1];
	require_non_empty(descriptor, left, 0);
	require_non_empty(descriptor, right, 1);

	// A vector is promoted for the duration of the multiplication: the
	// first operand gains a leading axis and the second a trailing one, so
	// that both are matrices. The axis added is dropped from the result.
	const auto left_is_vector = left.size() == 1;
	const auto right_is_vector = right.size() == 1;

	shape_type left_matrix = left;
	if (left_is_vector)
	{
		left_matrix.insert(left_matrix.begin(), 1);
	}
	shape_type right_matrix = right;
	if (right_is_vector)
	{
		right_matrix.push_back(1);
	}

	const auto rows = left_matrix[left_matrix.size() - 2];
	const auto inner = left_matrix.back();
	const auto other_inner = right_matrix[right_matrix.size() - 2];
	const auto columns = right_matrix.back();

	if (inner != other_inner)
	{
		std::ostringstream oss;
		oss << descriptor << ": input operand "
			<< describe_operand(descriptor, 0, false)
			<< " has " << inner << " columns, but input operand "
			<< describe_operand(descriptor, 1, false)
			<< " has " << other_inner << " rows.";
		throw std::invalid_argument(oss.str());
	}

	// Everything before the matrix is a stack, broadcast as elsewhere.
	shape_type shape(
		left_matrix.begin(),
		std::prev(left_matrix.end(), 2)
	);
	const shape_type right_batch(
		right_matrix.begin(),
		std::prev(right_matrix.end(), 2)
	);
	broadcast_extents_accumulate(shape, make_span(right_batch));

	if (!left_is_vector)
	{
		shape.push_back(rows);
	}
	if (!right_is_vector)
	{
		shape.push_back(columns);
	}

	assign_output_shapes(canonical_output_shapes, std::move(shape));
}

const matrix_multiply_shape_policy&
matrix_multiply_shape_policy::get() noexcept
{
	static const matrix_multiply_shape_policy instance;
	return instance;
}

} // namespace ops
} // namespace rexlib
