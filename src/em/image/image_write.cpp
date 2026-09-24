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

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

void write(
	const_array_ref arr,
	const std::string &path,
	const image_write_format_manager &manager,
	numerical_type data_type,
	image_metadata metadata
)
{
	if (data_type == numerical_type::unknown)
	{
		data_type = arr.get_descriptor().get_data_type();
	}

	std::vector<std::size_t> extents;
	arr.get_descriptor().get_layout().get_extents(extents);
	const auto rank = extents.size();

	const auto writer = manager.open(
		path,
		image_descriptor(make_span(extents), rank, data_type),
		metadata
	);

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
