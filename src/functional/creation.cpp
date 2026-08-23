// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/functional/creation.hpp>

#include <xmipp4/core/ndarray/array_descriptor.hpp>
#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/ops/assignment/copy_operation.hpp>
#include <xmipp4/ops/assignment/fill_operation.hpp>
#include <xmipp4/ops/creation/arange_operation.hpp>
#include <xmipp4/ops/creation/linspace_operation.hpp>
#include <xmipp4/core/dispatch/execute.hpp>
#include <xmipp4/core/dispatch/execution_context.hpp>
#include <xmipp4/core/binary/bit.hpp>
#include <xmipp4/core/hardware/device_context.hpp>
#include <xmipp4/core/hardware/device_session.hpp>
#include <xmipp4/core/hardware/device_properties.hpp>
#include <xmipp4/core/hardware/memory_allocator.hpp>
#include <xmipp4/core/hardware/buffer.hpp>

#include <core/logger.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace rex
{

namespace
{

std::shared_ptr<buffer> reuse_array_storage(
	array &donor,
	std::size_t target_size,
	const memory_resource &target_resource
)
{
	auto storage = donor.share_storage();
	if (!storage)
	{
		return nullptr;
	}

	if (&storage->get_memory_resource() != &target_resource)
	{
		REX_LOG_WARN(
			"empty: the output array storage lives on a different "
			"memory resource than the one requested by the affinity; "
			"reallocating on the requested resource."
		);
		return nullptr;
	}

	if (storage->get_size() < target_size)
	{
		return nullptr;
	}

	return storage;
}

std::shared_ptr<buffer> allocate_array_storage(
	std::size_t size,
	const device_context &device_context,
	memory_allocator &allocator
)
{
	const auto &session = device_context.get_device_session();
	REX_ASSERT(session);

	const auto &properties = session->get_properties();
	const auto &queue = device_context.get_active_queue();

	const auto max_alignment = allocator.get_max_alignment();
	const auto preferred_alignment = properties.get_optimal_data_alignment();
	const auto base_alignment = std::min(max_alignment, preferred_alignment);
	const auto alignment = std::min(base_alignment, bit_ceil(size));

	return allocator.allocate(size, alignment, queue.get());
}

array_descriptor make_sequence_descriptor(
	std::size_t count,
	numerical_type data_type
)
{
	return array_descriptor(
		strided_layout::make_contiguous_layout(make_span(&count, 1)),
		data_type
	);
}

bool is_whole_number(const scalar_value &value) noexcept
{
	const auto category = get_category(value.get_data_type());
	return category == numerical_type_category::signed_integer ||
	       category == numerical_type_category::unsigned_integer;
}

/**
 * @brief Count the whole numbers a half open range holds.
 *
 * The magnitudes are taken unsigned, so that a range spanning the whole of
 * the signed domain does not overflow on its way to a length that is
 * perfectly representable.
 */
std::size_t whole_number_range_length(
	std::int64_t start,
	std::int64_t stop,
	std::int64_t step
)
{
	std::uint64_t distance;
	std::uint64_t magnitude;
	if (step > 0)
	{
		if (stop <= start)
		{
			return 0;
		}
		distance = static_cast<std::uint64_t>(stop) -
		           static_cast<std::uint64_t>(start);
		magnitude = static_cast<std::uint64_t>(step);
	}
	else
	{
		if (stop >= start)
		{
			return 0;
		}
		distance = static_cast<std::uint64_t>(start) -
		           static_cast<std::uint64_t>(stop);
		// Negated in two halves, so that the most negative step, whose
		// magnitude the signed domain cannot hold, is not a special case.
		magnitude = static_cast<std::uint64_t>(-(step + 1)) + 1;
	}

	// Rounded up: the last element is the last one strictly before the stop.
	return static_cast<std::size_t>((distance + magnitude - 1) / magnitude);
}

/**
 * @brief Work out how many elements an arange writes.
 *
 * Whole numbers are counted as whole numbers, which keeps a purely integral
 * range exact however large its bounds are. Anything else is counted in
 * double precision, which is what the division it takes needs anyway.
 */
std::size_t compute_arange_length(
	const scalar_value &start,
	const scalar_value &stop,
	const scalar_value &step
)
{
	if (is_whole_number(start) && is_whole_number(stop) &&
	    is_whole_number(step))
	{
		const auto integral_step = scalar_value_cast<std::int64_t>(step);
		if (integral_step == 0)
		{
			throw std::invalid_argument(
				"arange: the step must not be zero, as a range it never "
				"advances through has no length."
			);
		}

		return whole_number_range_length(
			scalar_value_cast<std::int64_t>(start),
			scalar_value_cast<std::int64_t>(stop),
			integral_step
		);
	}

	const auto real_step = scalar_value_cast<float64_t>(step);
	if (real_step == 0.0)
	{
		throw std::invalid_argument(
			"arange: the step must not be zero, as a range it never "
			"advances through has no length."
		);
	}

	const auto length = std::ceil(
		(scalar_value_cast<float64_t>(stop) -
		 scalar_value_cast<float64_t>(start)) / real_step
	);

	// Written as a rejection of everything positive rather than as a test for
	// zero or less, so that a length that is not a number is caught here too.
	if (!(length > 0.0))
	{
		return 0;
	}
	if (!std::isfinite(length))
	{
		throw std::invalid_argument(
			"arange: the requested range does not have a finite length."
		);
	}

	return static_cast<std::size_t>(length);
}

} // anonymous namespace


array empty(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out
)
{
	const auto &device_context = context.get_device_context();
	const auto &allocator = device_context.get_allocator(affinity);
	if (!allocator)
	{
		throw std::invalid_argument(
			"empty: the execution context has no allocator for the requested "
			"affinity; cannot allocate array storage."
		);
	}

	std::shared_ptr<buffer> storage;
	const auto size = compute_storage_requirement(descriptor);
	if (out)
	{
		storage = reuse_array_storage(
			*out, 
			size, 
			allocator->get_memory_resource()
		);
	}

	if (!storage)
	{
		storage = allocate_array_storage(size, device_context, *allocator);
	}

	if (!out)
	{
		return array(std::move(storage), std::move(descriptor));
	}

	if (
		out->get_storage() != storage.get() ||
		out->get_descriptor() != descriptor
	)
	{
		*out = array(std::move(storage), std::move(descriptor));
	}

	return out->share();
}

array zeros(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out
)
{
	return full(
		descriptor,
		affinity,
		0,
		context,
		out
	);
}

array ones(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out
)
{
	return full(
		descriptor,
		affinity,
		1,
		context,
		out
	);
}

array full(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const scalar_value &fill_value,
	const execution_context &context,
	array *out
)
{
	std::array<array, 1> outputs = {
		empty(descriptor, affinity, context, out) 
	};

	execute(
		ops::fill_operation(fill_value),
		make_span(outputs),
		{},
		context
	);

	return std::move(outputs[0]);
}

array arange(
	const scalar_value &start,
	const scalar_value &stop,
	const scalar_value &step,
	numerical_type data_type,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out
)
{
	// Where the progression stops is resolved into a length here, where the
	// bounds are still numbers, because the array has to be allocated before
	// the operation that writes it can be dispatched.
	const auto count = compute_arange_length(start, stop, step);

	std::array<array, 1> outputs = {
		empty(
			make_sequence_descriptor(count, data_type),
			affinity,
			context,
			out
		)
	};

	execute(
		ops::arange_operation(start, step, count),
		make_span(outputs),
		{},
		context
	);

	return std::move(outputs[0]);
}

array arange(
	const scalar_value &start,
	const scalar_value &stop,
	numerical_type data_type,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out
)
{
	return arange(start, stop, 1, data_type, affinity, context, out);
}

array arange(
	const scalar_value &stop,
	numerical_type data_type,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out
)
{
	return arange(0, stop, 1, data_type, affinity, context, out);
}

array linspace(
	const scalar_value &start,
	const scalar_value &stop,
	std::size_t count,
	bool endpoint,
	numerical_type data_type,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out
)
{
	std::array<array, 1> outputs = {
		empty(
			make_sequence_descriptor(count, data_type),
			affinity,
			context,
			out
		)
	};

	execute(
		ops::linspace_operation(start, stop, count, endpoint),
		make_span(outputs),
		{},
		context
	);

	return std::move(outputs[0]);
}

array linspace(
	const scalar_value &start,
	const scalar_value &stop,
	std::size_t count,
	numerical_type data_type,
	memory_resource_affinity affinity,
	const execution_context &context,
	array *out
)
{
	return linspace(
		start,
		stop,
		count,
		true,
		data_type,
		affinity,
		context,
		out
	);
}

array copy(
	const_array_ref source,
	const execution_context &context,
	array *out
)
{
	return execute_unary(ops::copy_operation(), source, context, out);
}

void fill(
	array &out,
	const scalar_value &fill_value,
	const execution_context &context
)
{
	execute(
		ops::fill_operation(fill_value),
		make_span(&out, 1),
		{},
		context
	);
}

} // namespace rex
