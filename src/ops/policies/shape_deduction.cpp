// SPDX-License-Identifier: GPL-3.0-only

#include "shape_deduction.hpp"

#include <xmipp4/core/layout/broadcast.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace rex
{
namespace ops
{

shape_type broadcast_input_shapes(
	const operation_descriptor &descriptor,
	span<const shape_type> input_shapes
)
{
	if (input_shapes.empty())
	{
		std::ostringstream oss;
		oss << descriptor << ": at least one input operand is needed to "
			<< "take a shape from.";
		throw std::invalid_argument(oss.str());
	}

	shape_type result = input_shapes[0];
	for (std::size_t i = 1; i < input_shapes.size(); ++i)
	{
		broadcast_extents_accumulate(result, make_span(input_shapes[i]));
	}

	return result;
}

void assign_output_shapes(
	span<shape_type> canonical_output_shapes,
	shape_type shape
)
{
	if (canonical_output_shapes.empty())
	{
		return;
	}

	// The last slot is given the shape itself, every other one having been
	// served from it first.
	std::fill(
		canonical_output_shapes.begin(),
		std::prev(canonical_output_shapes.end()),
		shape
	);
	canonical_output_shapes.back() = std::move(shape);
}

} // namespace ops
} // namespace rex
