// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/strict_image_transfer_sanitizer.hpp>

#include <rexlib/em/image/image_transfer_shape.hpp>

#include <memory>
#include <stdexcept>

namespace rexlib
{
namespace em
{

namespace
{

void check_rank(
	std::size_t actual,
	std::size_t expected,
	const char *message
)
{
	if (actual != expected)
	{
		throw std::invalid_argument(message);
	}
}

// The extents of a shape cover the trailing axes of a side, which spans a
// single position along the leading ones.
bool fits(
	const image_transfer_shape &shape,
	span<const std::size_t> offset,
	span<const std::size_t> extents
) noexcept
{
	const auto rank = extents.size();
	for (std::size_t axis = 0; axis < rank; ++axis)
	{
		const auto position = offset[axis];
		const auto boundary = extents[axis];
		if (position > boundary ||
			shape.get_extent(rank, axis) > boundary - position)
		{
			return false;
		}
	}

	return true;
}

} // anonymous namespace

std::vector<image_transfer_plan> strict_image_transfer_sanitizer::sanitize(
	const image_transfer_plan &regions,
	span<const std::size_t> file_extents,
	span<const std::size_t> array_extents
) const
{
	const auto &shape = regions.get_shape();
	check_rank(
		file_extents.size(),
		shape.get_file_rank(),
		"strict_image_transfer_sanitizer: The file extents do not have the "
		"file rank of the regions."
	);
	check_rank(
		array_extents.size(),
		shape.get_array_rank(),
		"strict_image_transfer_sanitizer: The array extents do not have the "
		"array rank of the regions."
	);

	const auto count = regions.get_region_count();
	for (std::size_t i = 0; i < count; ++i)
	{
		if (!fits(shape, regions.get_file_offset(i), file_extents))
		{
			throw std::out_of_range(
				"strict_image_transfer_sanitizer: A region reaches past the "
				"file."
			);
		}

		if (!fits(shape, regions.get_array_offset(i), array_extents))
		{
			throw std::out_of_range(
				"strict_image_transfer_sanitizer: A region reaches past the "
				"array."
			);
		}
	}

	return std::vector<image_transfer_plan>(1, regions);
}

const std::shared_ptr<const strict_image_transfer_sanitizer>&
strict_image_transfer_sanitizer::get_shared()
{
	static const auto instance =
		std::make_shared<const strict_image_transfer_sanitizer>();
	return instance;
}

} // namespace em
} // namespace rexlib
