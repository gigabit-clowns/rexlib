// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/clipping_image_transfer_sanitizer.hpp>

#include <rexlib/em/image/image_transfer_shape.hpp>

#include <algorithm>
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

std::size_t get_remaining(
	span<const std::size_t> extents,
	span<const std::size_t> offset,
	std::size_t axis
) noexcept
{
	const auto boundary = extents[axis];
	const auto position = offset[axis];
	return position < boundary ? boundary - position : 0;
}

// The extents of a plan cover the trailing axes of a side, which spans a
// single position along the leading ones. Those leading axes cannot be
// shortened, so a region that starts past one of them transfers nothing.
bool clip_region(
	const image_transfer_plan &regions,
	std::size_t region_index,
	span<const std::size_t> file_extents,
	span<const std::size_t> array_extents,
	std::vector<std::size_t> &extents
)
{
	const auto &shape = regions.get_shape();
	const auto rank = shape.get_rank();
	const auto file_offset = regions.get_file_offset(region_index);
	const auto array_offset = regions.get_array_offset(region_index);
	const auto file_leading = shape.get_leading_rank(shape.get_file_rank());
	const auto array_leading =
		shape.get_leading_rank(shape.get_array_rank());

	for (std::size_t axis = 0; axis < file_leading; ++axis)
	{
		if (get_remaining(file_extents, file_offset, axis) == 0)
		{
			return false;
		}
	}

	for (std::size_t axis = 0; axis < array_leading; ++axis)
	{
		if (get_remaining(array_extents, array_offset, axis) == 0)
		{
			return false;
		}
	}

	const auto whole = shape.get_extents();
	extents.assign(whole.begin(), whole.end());
	for (std::size_t axis = 0; axis < rank; ++axis)
	{
		auto &extent = extents[axis];
		extent = std::min(
			extent,
			get_remaining(file_extents, file_offset, file_leading + axis)
		);
		extent = std::min(
			extent,
			get_remaining(array_extents, array_offset, array_leading + axis)
		);

		if (extent == 0)
		{
			return false;
		}
	}

	return true;
}

std::size_t find_plan(
	const std::vector<image_transfer_plan> &plans,
	const std::vector<std::size_t> &extents
) noexcept
{
	for (std::size_t i = 0; i < plans.size(); ++i)
	{
		const auto candidate = plans[i].get_shape().get_extents();
		if (std::equal(
				candidate.begin(),
				candidate.end(),
				extents.cbegin(),
				extents.cend()
			))
		{
			return i;
		}
	}

	return plans.size();
}

} // anonymous namespace

std::vector<image_transfer_plan> clipping_image_transfer_sanitizer::sanitize(
	const image_transfer_plan &regions,
	span<const std::size_t> file_extents,
	span<const std::size_t> array_extents
) const
{
	const auto &shape = regions.get_shape();
	check_rank(
		file_extents.size(),
		shape.get_file_rank(),
		"clipping_image_transfer_sanitizer: The file extents do not have the "
		"file rank of the regions."
	);
	check_rank(
		array_extents.size(),
		shape.get_array_rank(),
		"clipping_image_transfer_sanitizer: The array extents do not have "
		"the array rank of the regions."
	);

	std::vector<image_transfer_plan> result;
	std::vector<std::size_t> extents;

	const auto count = regions.get_region_count();
	for (std::size_t i = 0; i < count; ++i)
	{
		if (!clip_region(regions, i, file_extents, array_extents, extents))
		{
			continue;
		}

		const auto plan = find_plan(result, extents);
		if (plan == result.size())
		{
			result.emplace_back(
				image_transfer_shape(
					extents,
					shape.get_file_rank(),
					shape.get_array_rank()
				)
			);
		}

		result[plan].add(
			regions.get_file_offset(i),
			regions.get_array_offset(i)
		);
	}

	return result;
}

const std::shared_ptr<const clipping_image_transfer_sanitizer>&
clipping_image_transfer_sanitizer::get_shared()
{
	static const auto instance =
		std::make_shared<const clipping_image_transfer_sanitizer>();
	return instance;
}

} // namespace em
} // namespace rexlib
