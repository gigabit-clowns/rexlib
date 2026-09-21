// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_batch_sink.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_sink.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>

#include <em/image/image_batch_plan.hpp>

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

image_batch_sink::image_batch_sink(std::shared_ptr<const image_sink> sink)
	: m_sink(std::move(sink))
{
	if (!m_sink)
	{
		throw std::invalid_argument(
			"image_batch_sink: The downstream image sink must not be null."
		);
	}
}

image_batch_sink::~image_batch_sink() = default;

std::shared_ptr<completion> image_batch_sink::write(
	const_array source,
	span<const image_location> locations
) const
{
	std::vector<std::size_t> array_extents;
	source.get_descriptor().get_layout().get_extents(array_extents);

	const auto transaction = make_batch_transaction_plan(
		make_span(array_extents),
		locations,
		"image_batch_sink::write"
	);

	REXLIB_ASSERT(m_sink);
	return m_sink->write(std::move(source), transaction);
}

} // namespace em
} // namespace rexlib
