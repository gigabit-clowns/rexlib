// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_write.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_sink.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_writer.hpp>

#include <em/image/image_batch_plan.hpp>

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

namespace
{

std::vector<std::size_t> extents_of(const const_array_ref &arr)
{
	std::vector<std::size_t> extents;
	arr.get_descriptor().get_layout().get_extents(extents);
	return extents;
}

numerical_type resolve_data_type(
	const const_array_ref &arr,
	numerical_type data_type
) noexcept
{
	return data_type == numerical_type::unknown
		? arr.get_descriptor().get_data_type()
		: data_type;
}

} // anonymous namespace

void write_single(
	const_array_ref arr,
	const std::string &path,
	const image_write_format_manager &manager,
	numerical_type data_type,
	const image_metadata &metadata
)
{
	const auto extents = extents_of(arr);
	if (extents.empty())
	{
		throw std::invalid_argument("write_single: The array has no extents.");
	}

	const image_descriptor descriptor(
		make_span(extents),
		extents.size(),
		resolve_data_type(arr, data_type)
	);

	write(arr, path, manager, descriptor, metadata);
}

void write_stack(
	const_array_ref arr,
	const std::string &path,
	const image_write_format_manager &manager,
	numerical_type data_type,
	const image_metadata &metadata
)
{
	const auto extents = extents_of(arr);
	if (extents.size() < 2)
	{
		throw std::invalid_argument(
			"write_stack: The array needs a leading extent to stack along "
			"and at least one more for each image or volume."
		);
	}

	const image_descriptor descriptor(
		make_span(extents),
		extents.size() - 1,
		resolve_data_type(arr, data_type)
	);

	write(arr, path, manager, descriptor, metadata);
}

void write(
	const_array_ref arr,
	const std::string &path,
	const image_write_format_manager &manager,
	const image_descriptor &descriptor,
	const image_metadata &metadata
)
{
	const auto extents = extents_of(arr);

	const auto file_extents = descriptor.get_extents();
	if (!std::equal(
			extents.cbegin(),
			extents.cend(),
			file_extents.begin(),
			file_extents.end()
		))
	{
		throw std::invalid_argument(
			"write: The extents of the array are not those of the "
			"descriptor."
		);
	}

	const auto writer = manager.open(path, descriptor, metadata);

	const auto rank = extents.size();
	image_transfer_plan plan(make_span(extents), rank, rank);
	const std::vector<std::size_t> origin(rank, 0UL);
	plan.add(make_span(origin), make_span(origin));

	writer->write(arr, plan);
	writer->flush();
}

std::shared_ptr<completion> write_batch_async(
	const image_sink &sink,
	const_array source,
	span<const image_location> locations
)
{
	std::vector<std::size_t> array_extents;
	source.get_descriptor().get_layout().get_extents(array_extents);

	const auto transaction = make_batch_plan(
		make_span(array_extents),
		locations,
		"write_batch_async"
	);

	return sink.write(std::move(source), transaction);
}

} // namespace em
} // namespace rexlib
