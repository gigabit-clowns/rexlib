// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_patch_source.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/counting_completion.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_source.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/index_table.hpp>

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

namespace
{

// A patch spans from its centre less half its extent, which may begin before
// the image does. The part before it is carried by the array offset instead
// of by a negative file offset, so that the region the source is handed keeps
// the extents of a whole patch and every patch of the batch shares them.
void place_patch(
	span<const std::size_t> position,
	span<const std::size_t> patch_extents,
	std::size_t file_leading,
	std::vector<std::size_t> &file_offset,
	std::vector<std::size_t> &array_offset
) noexcept
{
	for (std::size_t axis = 0; axis < patch_extents.size(); ++axis)
	{
		const auto half =
			static_cast<std::ptrdiff_t>(patch_extents[axis] / 2);
		const auto corner =
			static_cast<std::ptrdiff_t>(position[axis]) - half;

		file_offset[file_leading + axis] =
			static_cast<std::size_t>(std::max<std::ptrdiff_t>(corner, 0));
		array_offset[1 + axis] =
			static_cast<std::size_t>(std::max<std::ptrdiff_t>(-corner, 0));
	}
}

} // anonymous namespace

image_patch_source::image_patch_source(
	std::shared_ptr<const image_source> source
)
	: m_source(std::move(source))
{
	if (!m_source)
	{
		throw std::invalid_argument(
			"image_patch_source: The downstream image source must not be "
			"null."
		);
	}
}

image_patch_source::~image_patch_source() = default;

std::shared_ptr<completion> image_patch_source::read(
	array destination,
	const image_location &location,
	const index_table &positions
) const
{
	std::vector<std::size_t> array_extents;
	destination.get_descriptor().get_layout().get_extents(array_extents);

	if (array_extents.empty())
	{
		throw std::invalid_argument(
			"image_patch_source::read: destination array has no extents. "
			"Expected patch_rank+1 extents"
		);
	}

	const auto batch_size = positions.get_index_count();
	if (array_extents.front() != batch_size)
	{
		throw std::invalid_argument(
			"image_patch_source::read: destination array's first dimension "
			"(batch size) must match the position count."
		);
	}

	const auto array_rank = array_extents.size();
	const auto patch_rank = array_rank - 1;
	if (positions.get_rank() != patch_rank)
	{
		throw std::invalid_argument(
			"image_patch_source::read: the positions must have the rank of "
			"one patch, which is one less than that of the destination."
		);
	}

	if (batch_size == 0)
	{
		return std::make_shared<counting_completion>(0);
	}

	const auto stack_indexing = location.has_position();
	const auto file_rank = stack_indexing ? array_rank : patch_rank;
	const auto file_leading = file_rank - patch_rank;
	const span<const std::size_t> patch_extents(
		array_extents.data() + 1,
		array_extents.size() - 1
	);

	image_transaction_plan transaction(
		patch_extents,
		file_rank,
		array_rank
	);

	transaction.reserve(1, batch_size);
	const auto file_index = transaction.add_file(location.get_path());
	std::vector<std::size_t> file_offset(file_rank, 0UL);
	std::vector<std::size_t> array_offset(array_rank, 0UL);
	if (stack_indexing)
	{
		file_offset[0] = location.get_position_in_stack();
	}

	for (std::size_t i = 0; i < batch_size; ++i)
	{
		array_offset[0] = i;
		place_patch(
			positions.get(i),
			patch_extents,
			file_leading,
			file_offset,
			array_offset
		);

		transaction.add(
			file_index,
			make_span(file_offset),
			make_span(array_offset)
		);
	}

	REXLIB_ASSERT(m_source);
	return m_source->read(std::move(destination), transaction);
}

} // namespace em
} // namespace rexlib
