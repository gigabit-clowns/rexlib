// SPDX-License-Identifier: GPL-3.0-only

#include <rex/functional/transfer.hpp>

#include <rex/core/dispatch/execution_context.hpp>
#include <rex/core/dispatch/execute.hpp>
#include <rex/functional/creation.hpp>
#include <rex/core/ndarray/array_descriptor.hpp>
#include <rex/ops/assignment/copy_operation.hpp>
#include <rex/core/hardware/buffer.hpp>
#include <rex/core/hardware/memory_allocator.hpp>

namespace rexlib
{

array transfer(
	array &input,
	memory_resource_affinity affinity,
	const execution_context &context
)
{
	const auto *storage = input.get_storage();
	if (!storage)
	{
		throw std::invalid_argument(
			"transfer: Expected an input with associated storage."
		);
	}

	const auto &allocator = 
		context.get_device_context().get_allocator(affinity);
	if (!allocator)
	{
		throw std::invalid_argument(
			"transfer: Expected a device context with an allocator for the "
			"requested affinity."
		);
	}

	const auto &source_resource = storage->get_memory_resource();
	const auto &target_resource = allocator->get_memory_resource();
	if (&source_resource == &target_resource)
	{
		return input.share();
	}

	return transfer_copy(input, affinity, context, nullptr);
}

array transfer_copy(
	const_array_ref input,
	memory_resource_affinity affinity,
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
			input_descriptor.get_data_type()
		),
		affinity,
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

array to_device(
	array &input,
	const execution_context &context
)
{
	return transfer(
		input,
		memory_resource_affinity::device,
		context
	);
}

array to_device_copy(
	const_array_ref input,
	const execution_context &context,
	array *out
)
{
	return transfer_copy(
		std::move(input), 
		memory_resource_affinity::device, 
		context, 
		out
	);
}

array to_host(
	array &input,
	const execution_context &context
)
{
	return transfer(
		input,
		memory_resource_affinity::host,
		context
	);
}

array to_host_copy(
	const_array_ref input,
	const execution_context &context,
	array *out
)
{
	return transfer_copy(
		std::move(input), 
		memory_resource_affinity::host, 
		context, 
		out
	);
}

} // namespace rexlib
