// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/dispatch/operation_data_type_policy.hpp>

#include <rexlib/core/platform/assert.hpp>

#include <stdexcept>
#include <sstream>

namespace rexlib
{

operation_data_type_policy::operation_data_type_policy() noexcept = default;
operation_data_type_policy::~operation_data_type_policy() = default;

void operation_data_type_policy::accept(
	const operation_descriptor &descriptor,
	span<const numerical_type> user_output_types,
	span<const numerical_type> canonical_output_types,
	span<const numerical_type> /*input_types*/
) const
{
	REXLIB_ASSERT(user_output_types.size() == canonical_output_types.size());

	for (std::size_t i = 0; i < user_output_types.size(); ++i)
	{
		if (user_output_types[i] != canonical_output_types[i])
		{
			std::ostringstream oss;
			oss << descriptor << ": output operand "
				<< describe_operand(descriptor, i, true)
				<< " has data type " << user_output_types[i]
				<< ", but this operation deduces "
				<< canonical_output_types[i] << " from its inputs.";
			throw std::invalid_argument(oss.str());
		}
	}
}

} // namespace rexlib
