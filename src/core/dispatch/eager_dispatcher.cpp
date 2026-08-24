// SPDX-License-Identifier: GPL-3.0-only

#include "eager_dispatcher.hpp"

#include <rex/core/binary/bit.hpp>
#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>
#include <rex/core/dispatch/operand_signature.hpp>
#include <rex/core/dispatch/operation.hpp>
#include <rex/core/dispatch/program_manager.hpp>
#include <rex/core/hardware/program.hpp>
#include <rex/core/hardware/command.hpp>
#include <rex/core/hardware/command_queue.hpp>
#include <rex/core/hardware/program_scratch_requirement.hpp>
#include <rex/core/hardware/memory_allocator.hpp>
#include <rex/core/hardware/device_session.hpp>
#include <rex/core/hardware/device_properties.hpp>
#include <rex/core/hardware/device_context.hpp>
#include <rex/core/hardware/buffer.hpp>

#include <core/logger.hpp>
#include <core/config.hpp>
#include <core/hardware/memory_allocator_table.hpp>

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <boost/container/small_vector.hpp>

namespace rex
{

namespace
{

template <std::size_t N>
boost::container::small_vector<array_descriptor, N>
extract_input_descriptors(
	span<const const_array_ref> input_operands,
	std::integral_constant<std::size_t, N> /*small_cap_tag*/
)
{
	boost::container::small_vector<array_descriptor, N> result;
	result.reserve(input_operands.size());
	for (const auto &input_operand : input_operands)
	{
		result.push_back(input_operand.get_descriptor());
	}
	return result;
}
template <std::size_t N>
boost::container::small_vector<numerical_type, N>
extract_data_types(
	span<const array_descriptor> descriptors,
	std::integral_constant<std::size_t, N> /*small_cap_tag*/
)
{
	boost::container::small_vector<numerical_type, N> result;
	result.reserve(descriptors.size());
	for (const auto &d : descriptors)
	{
		result.push_back(d.get_data_type());
	}
	return result;
}

template <std::size_t N>
boost::container::small_vector<operation_shape_policy::shape_type, N>
extract_shapes(
	span<const array_descriptor> descriptors,
	std::integral_constant<std::size_t, N> /*small_cap_tag*/
)
{
	boost::container::small_vector<operation_shape_policy::shape_type, N>
		result;
	result.reserve(descriptors.size());
	for (const auto &d : descriptors)
	{
		operation_shape_policy::shape_type shape;
		d.get_layout().get_extents(shape);
		result.push_back(std::move(shape));
	}
	return result;
}

template <std::size_t N>
boost::container::small_vector<operation_shape_policy::shape_type, N>
make_empty_shapes(
	std::size_t count,
	std::integral_constant<std::size_t, N> /*small_cap_tag*/
)
{
	return boost::container::small_vector<
		operation_shape_policy::shape_type, N
	>(count);
}

template <std::size_t N>
boost::container::small_vector<numerical_type, N>
make_empty_data_types(
	std::size_t count,
	std::integral_constant<std::size_t, N> /*small_cap_tag*/
)
{
	return boost::container::small_vector<numerical_type, N>(count);
}

template <std::size_t N>
boost::container::small_vector<array_descriptor, N>
resolve_output_descriptors(
	const operation_descriptor &descriptor,
	span<const array> output_operands,
	span<const operation_shape_policy::shape_type> canonical_shapes,
	span<const numerical_type> canonical_data_types,
	bool &needs_acceptance,
	std::integral_constant<std::size_t, N> /*small_cap_tag*/
)
{
	const auto n = output_operands.size();
	REXLIB_ASSERT(canonical_shapes.size() == n);
	REXLIB_ASSERT(canonical_data_types.size() == n);

	boost::container::small_vector<array_descriptor, N> result;
	result.reserve(n);
	for (std::size_t i = 0; i < n; ++i)
	{
		const auto &output_operand = output_operands[i];

		if (output_operand.get_storage())
		{
			result.push_back(output_operand.get_descriptor());
			needs_acceptance = true;
		}
		else
		{
			const auto &canonical_shape = canonical_shapes[i];
			const auto canonical_data_type = canonical_data_types[i];

			// A rule whose output type is free leaves it undetermined for a
			// user supplied output to settle. With nothing supplied there is
			// nothing to settle it, and sizing a buffer from an unknown type
			// would allocate whatever get_size() happens to return.
			if (canonical_data_type == numerical_type::unknown)
			{
				std::ostringstream oss;
				oss << descriptor << ": the data type of output operand "
					<< describe_operand(descriptor, i, true)
					<< " is not determined by the inputs, so it must be "
					<< "supplied as a pre-allocated output.";
				throw std::invalid_argument(oss.str());
			}

			result.emplace_back(
				strided_layout::make_contiguous_layout(
					make_span(canonical_shape)
				),
				canonical_data_type
			);
		}
	}

	REXLIB_ASSERT(result.size() == n);
	return result;
}

template <std::size_t N>
boost::container::small_vector<std::shared_ptr<buffer>, N>
resolve_output_storage(
	span<array> output_operands,
	span<const array_descriptor> descriptors,
	const device_context &device_context,
	command_queue &queue,
	std::integral_constant<std::size_t, N> /*small_cap_tag*/
)
{
	using result_type = boost::container::small_vector<
		std::shared_ptr<buffer>,
		N
	>;

	const auto n = output_operands.size();
	REXLIB_ASSERT(n == descriptors.size());

	result_type result;
	result.reserve(n);

	const auto &session = device_context.get_device_session();
	const auto &allocator = device_context.get_allocator(
		memory_resource_affinity::device
	);
	const auto &properties = session->get_properties();
	const auto max_alignment = allocator->get_max_alignment();
	const auto preferred_alignment = properties.get_optimal_data_alignment();
	const auto base_alignment = std::min(max_alignment, preferred_alignment);

	for (std::size_t i = 0; i < n; ++i)
	{
		auto &output_operand = output_operands[i];
		const auto &descriptor = descriptors[i];

		auto storage = output_operand.share_storage();
		if (!storage)
		{
			const auto size = compute_storage_requirement(descriptor);
			const auto alignment = std::min(
				base_alignment,
				bit_ceil(size)
			);
			storage = allocator->allocate(size, alignment, &queue);
			output_operand = array(storage, descriptor); // Store in output.
		}

		REXLIB_ASSERT(storage);
		REXLIB_ASSERT(output_operand.get_descriptor() == descriptor);
		result.push_back(std::move(storage));
	}

	return result;
}

template <std::size_t N>
boost::container::small_vector<std::shared_ptr<const buffer>, N>
extract_input_storage(
	span<const const_array_ref> operands,
	std::integral_constant<std::size_t, N> /*small_cap_tag*/
)
{
	using result_type = boost::container::small_vector<
		std::shared_ptr<const buffer>,
		N
	>;
	result_type result(operands.size());

	for (std::size_t i = 0; i < operands.size(); ++i)
	{
		result[i] = operands[i].share_storage();
		if (!result[i])
		{
			std::ostringstream oss;
			oss << "eager_dispatcher::dispatch: Input operand at "
				<< "index " << i << " does not have associated storage.";
			
			throw std::invalid_argument(oss.str());
		}
	}

	return result;
}

template <typename Ptr, std::size_t N>
boost::container::small_vector<operand_signature, N>
create_signatures(
	boost::container::small_vector<array_descriptor, N> &&descriptors,
	span<Ptr> storages
)
{
	const auto n = descriptors.size();
	REXLIB_ASSERT(n == storages.size());

	boost::container::small_vector<operand_signature, N> result(n);
	for (std::size_t i = 0; i < n; ++i)
	{
		REXLIB_ASSERT(storages[i]);
		result[i] = operand_signature(
			std::move(descriptors[i]), // Steal descriptors
			&(storages[i]->get_memory_resource())
		);
	}

	return result;
}

void validate_arity(
	const operation_arity& user,
	const operation_arity& expected
)
{
	if (user.get_output_count() != expected.get_output_count())
	{
		std::ostringstream oss;
		oss << "Expected " << expected.get_output_count()
			<< " output(s), but " << user.get_output_count() << " provided.";
		throw std::invalid_argument(oss.str());
	}

	if (user.get_input_count() != expected.get_input_count())
	{
		std::ostringstream oss;
		oss << "Expected " << expected.get_input_count()
			<< " input(s), but " << user.get_input_count() << " provided.";
		throw std::invalid_argument(oss.str());
	}
}

template <std::size_t N>
boost::container::small_vector<std::shared_ptr<buffer>, N>
allocate_scratch(
	span<const program_scratch_requirement> requirements,
	const device_context &device_context,
	command_queue &queue,
	std::integral_constant<std::size_t, N> /*small_cap_tag*/
)
{
	using result_type = boost::container::small_vector<
		std::shared_ptr<buffer>,
		N
	>;

	result_type result(requirements.size());

	for (std::size_t i = 0; i < requirements.size(); ++i)
	{
		const auto &requirement = requirements[i];
		const auto size = requirement.get_size();
		const auto alignment = requirement.get_alignment();
		const auto affinity = requirement.get_affinity();
		const auto &allocator = device_context.get_allocator(affinity);
		result[i] = allocator->allocate(size, alignment, &queue);
	}

	return result;
}
} // anonymous namespace

eager_dispatcher::eager_dispatcher(
	std::shared_ptr<const program_manager> program_manager,
	std::size_t cache_capacity
)
	: m_program_manager(std::move(program_manager))
	, m_program_cache(cache_capacity)
{
	if (!m_program_manager)
	{
		throw std::invalid_argument(
			"program_manager cannot be null"
		);
	}
}

eager_dispatcher::~eager_dispatcher() = default;

void eager_dispatcher::dispatch(
	const operation &op,
	span<array> output_operands,
	span<const const_array_ref> input_operands,
	const device_context &device_context
)
{
	using small_output_size_tag =
		std::integral_constant<std::size_t, REXLIB_SMALL_OUTPUT_OPERAND_COUNT>;
	using small_input_size_tag =
		std::integral_constant<std::size_t, REXLIB_SMALL_INPUT_OPERAND_COUNT>;
	using small_scratch_size_tag =
		std::integral_constant<std::size_t, REXLIB_SMALL_SCRATCH_OPERAND_COUNT>;

	const auto &queue = device_context.get_active_queue();
	if (!queue)
	{
		throw std::invalid_argument(
			"Expected a device context with a dereferenceable active queue."
		);
	}
	
	const auto n_outputs = output_operands.size();
	const auto n_inputs = input_operands.size();
	validate_arity(
		operation_arity(n_outputs, n_inputs),
		op.get_arity()
	);

	auto input_descriptors = extract_input_descriptors(
		input_operands,
		small_input_size_tag()
	);
	auto input_shapes = extract_shapes(
		make_span(input_descriptors.data(), n_inputs),
		small_input_size_tag()
	);
	auto input_data_types = extract_data_types(
		make_span(input_descriptors.data(), n_inputs),
		small_input_size_tag()
	);

	const auto &shape_policy = op.get_operation_shape_policy();
	const auto &data_type_policy = op.get_operation_data_type_policy();
	const auto &descriptor = op.get_descriptor();

	auto canonical_output_shapes =
		make_empty_shapes(n_outputs, small_output_size_tag());
	auto canonical_output_data_types =
		make_empty_data_types(n_outputs, small_output_size_tag());
	shape_policy.deduce(
		descriptor,
		make_span(canonical_output_shapes.data(), n_outputs),
		make_span(input_shapes.data(), n_inputs)
	);
	data_type_policy.deduce(
		descriptor,
		make_span(canonical_output_data_types.data(), n_outputs),
		make_span(input_data_types.data(), n_inputs)
	);

	bool needs_acceptance = false;
	auto output_descriptors = resolve_output_descriptors(
		descriptor,
		output_operands,
		make_span(canonical_output_shapes.data(), n_outputs),
		make_span(canonical_output_data_types.data(), n_outputs),
		needs_acceptance,
		small_output_size_tag()
	);

	if (needs_acceptance)
	{
		auto output_shapes = extract_shapes(
			make_span(output_descriptors.data(), n_outputs),
			small_output_size_tag()
		);
		auto output_data_types = extract_data_types(
			make_span(output_descriptors.data(), n_outputs),
			small_output_size_tag()
		);

		shape_policy.accept(
			descriptor,
			make_span(output_shapes.data(), n_outputs),
			make_span(canonical_output_shapes.data(), n_outputs),
			make_span(input_shapes.data(), n_inputs)
		);
		data_type_policy.accept(
			descriptor,
			make_span(output_data_types.data(), n_outputs),
			make_span(canonical_output_data_types.data(), n_outputs),
			make_span(input_data_types.data(), n_inputs)
		);
	}

	auto input_storages = extract_input_storage(
		input_operands,
		small_input_size_tag()
	);

	auto output_storages = resolve_output_storage(
		output_operands,
		make_span(output_descriptors.data(), n_outputs),
		device_context,
		*queue,
		small_output_size_tag()
	);

	auto output_signatures = create_signatures(
		std::move(output_descriptors), // No longer needed.
		make_span(output_storages.data(), n_outputs)
	);
	auto input_signatures = create_signatures(
		std::move(input_descriptors),  // No longer needed.
		make_span(input_storages.data(), n_inputs)
	);

	REXLIB_ASSERT(m_program_manager);
	auto prog = m_program_manager->build(
		op,
		make_span(output_signatures.data(), n_outputs),
		make_span(input_signatures.data(), n_inputs),
		*queue,
		&m_program_cache
	);
	REXLIB_ASSERT(prog);

	auto scratch = allocate_scratch(
		prog->get_scratch_requirements(),
		device_context,
		*queue,
		small_scratch_size_tag()
	);

	command cmd(std::move(prog));
	cmd.bind_outputs(make_span(output_storages.data(), n_outputs));
	cmd.bind_inputs(make_span(input_storages.data(), n_inputs));
	cmd.bind_scratch(make_span(scratch.data(), scratch.size()));

	queue->submit(cmd);
}

// Declared in dispatcher.hpp.
std::shared_ptr<dispatcher> make_eager_dispatcher(
	std::shared_ptr<const program_manager> program_manager
)
{
	return std::make_shared<eager_dispatcher>(
		std::move(program_manager),
		REXLIB_DEFAULT_OPERATION_PROGRAM_CACHE_CAPACITY
	);
}

} // namespace rex
