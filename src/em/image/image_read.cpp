// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_read.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/clipping_image_transfer_sanitizer.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_loader.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>
#include <rexlib/em/image/index_table.hpp>
#include <rexlib/em/image/strict_image_transfer_sanitizer.hpp>
#include <rexlib/functional/creation.hpp>

#include <em/image/image_plan_builders.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

array read(
	const std::string &path,
	image_reader_provider &readers,
	const execution_context &context,
	numerical_type data_type
)
{
	return read(image_location(path), readers, context, data_type);
}

array read(
	const image_location &location,
	image_reader_provider &readers,
	const execution_context &context,
	numerical_type data_type
)
{
	const auto reader = readers.acquire(location.get_path());
	const auto &descriptor = reader->get_descriptor();
	const auto plan = make_location_plan(descriptor, location);

	auto destination = empty(
		make_contiguous_array_descriptor(
			plan.get_shape().get_extents(),
			data_type == numerical_type::unknown
				? descriptor.get_data_type()
				: data_type
		),
		memory_resource_affinity::host,
		context
	);

	array_ref destination_ref(destination);
	reader->read(destination_ref, plan);

	return destination;
}

std::shared_ptr<completion> read_batch_async(
	const image_loader &loader,
	array destination,
	span<const image_location> locations
)
{
	std::vector<std::size_t> array_extents;
	destination.get_descriptor().get_layout().get_extents(array_extents);

	const auto transaction =
		make_batch_plan(make_span(array_extents), locations);

	return loader.load(
		std::move(destination),
		transaction,
		strict_image_transfer_sanitizer::get_shared()
	);
}

std::shared_ptr<completion> read_patches_async(
	const image_loader &loader,
	array destination,
	const image_location &location,
	const index_table &centres
)
{
	std::vector<std::size_t> array_extents;
	destination.get_descriptor().get_layout().get_extents(array_extents);

	const auto transaction =
		make_patch_plan(make_span(array_extents), location, centres);

	return loader.load(
		std::move(destination),
		transaction,
		clipping_image_transfer_sanitizer::get_shared()
	);
}

} // namespace em
} // namespace rexlib
