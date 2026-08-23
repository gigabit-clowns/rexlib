// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/functional/cast.hpp>

#include <xmipp4/functional/creation.hpp>
#include <xmipp4/core/ndarray/array_descriptor.hpp>
#include <xmipp4/core/dispatch/execute.hpp>
#include <xmipp4/ops/assignment/copy_operation.hpp>
#include <xmipp4/core/hardware/memory_resource_affinity.hpp>

namespace rex
{

array cast(
	array &input,
	numerical_type target_type,
	const execution_context &context
)
{
	const auto source_type = input.get_descriptor().get_data_type();
	if (source_type == target_type)
	{
		return input.share();
	}

	return cast_copy(
		input,
		target_type,
		context,
		nullptr
	);
}

array cast_copy(
	const_array_ref input,
	numerical_type target_type,
	const execution_context &context,
	array *out
)
{
	const auto &input_descriptor = input.get_descriptor();

	std::vector<std::size_t> input_extents;
	input_descriptor.get_layout().get_extents(input_extents);

	array result = empty(
		array_descriptor(
			strided_layout::make_contiguous_layout(
				make_span(input_extents)
			),
			target_type
		),
		memory_resource_affinity::device,
		context,
		out
	);

	execute(
		ops::copy_operation(),
		make_span(&result, 1),
		make_span(&input, 1),
		context
	);

	return result;
}

} // namespace rex
