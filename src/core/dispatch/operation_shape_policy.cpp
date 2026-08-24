// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/dispatch/operation_shape_policy.hpp>

#include <rex/core/dispatch/operation_descriptor.hpp>
#include <rex/core/platform/assert.hpp>

#include <stdexcept>
#include <sstream>

namespace rex
{

operation_shape_policy::operation_shape_policy() noexcept = default;
operation_shape_policy::~operation_shape_policy() = default;

void operation_shape_policy::accept(
	const operation_descriptor &descriptor,
	span<const shape_type> user_output_shapes,
	span<const shape_type> canonical_output_shapes,
	span<const shape_type> /*input_shapes*/
) const
{
	REXLIB_ASSERT(user_output_shapes.size() == canonical_output_shapes.size());

	for (std::size_t i = 0; i < user_output_shapes.size(); ++i)
	{
		if (user_output_shapes[i] != canonical_output_shapes[i])
		{
			std::ostringstream oss;
			oss << descriptor << ": output operand "
				<< describe_operand(descriptor, i, true)
				<< " does not have the shape deduced from the inputs.";
			throw std::invalid_argument(oss.str());
		}
	}
}

} // namespace rex
