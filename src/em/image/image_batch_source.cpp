// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_batch_source.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_source.hpp>
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

image_batch_source::image_batch_source(
	std::shared_ptr<const image_source> source
)
	: m_source(std::move(source))
{
	if (!m_source)
	{
		throw std::invalid_argument(
			"image_batch_source: The downstream image source must not be null."
		);
	}
}

image_batch_source::~image_batch_source() = default;

std::shared_ptr<completion> image_batch_source::read(
	array destination,
	span<const image_location> locations
) const
{
	std::vector<std::size_t> array_extents;
	destination.get_descriptor().get_layout().get_extents(array_extents);

	const auto transaction = make_batch_transaction_plan(
		make_span(array_extents),
		locations,
		"image_batch_source::read"
	);

	REXLIB_ASSERT(m_source);
	return m_source->read(std::move(destination), transaction);
}

} // namespace em
} // namespace rexlib
