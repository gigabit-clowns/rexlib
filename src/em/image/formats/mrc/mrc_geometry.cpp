// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_geometry.hpp"

#include "mrc_constants.hpp"

#include <core/logger.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include <algorithm>
#include <functional>
#include <numeric>

namespace rexlib
{
namespace em
{
namespace mrc
{

namespace
{

std::size_t to_extent(std::int32_t count) noexcept
{
	return static_cast<std::size_t>(count);
}

enum class file_kind
{
	image,
	image_stack,
	volume,
	volume_stack
};

file_kind derive_file_kind(const mrc_header &header) noexcept
{
	const auto space_group = header.get_space_group();
	const auto sections = header.get_section_count();

	if (is_volume_stack_space_group(space_group))
	{
		const auto depth = header.get_section_sampling();
		if (depth == sections)
		{
			return file_kind::volume;
		}

		if (depth != 1)
		{
			return file_kind::volume_stack;
		}
	}
	else if (space_group != image_stack_space_group)
	{
		return file_kind::volume;
	}

	return sections == 1 ? file_kind::image : file_kind::image_stack;
}

std::vector<std::size_t> derive_stored_extents(const mrc_header &header)
{
	const auto kind = derive_file_kind(header);
	const auto columns = to_extent(header.get_column_count());
	const auto rows = to_extent(header.get_row_count());
	const auto sections = to_extent(header.get_section_count());

	if (kind == file_kind::volume_stack)
	{
		const auto depth = to_extent(header.get_section_sampling());
		return {sections / depth, depth, rows, columns};
	}

	if (kind == file_kind::image)
	{
		return {rows, columns};
	}

	return {sections, rows, columns}; // volume or image_stack
}

std::size_t derive_core_rank(const mrc_header &header) noexcept
{
	switch (derive_file_kind(header))
	{
	case file_kind::image:
	case file_kind::image_stack:
		return 2;
	default: // file_kind::volume or file_kind::volume_stack
		return 3;
	}
}

std::vector<std::ptrdiff_t>
derive_stored_strides(const std::vector<std::size_t> &extents)
{
	std::vector<std::ptrdiff_t> strides(extents.size());

	std::ptrdiff_t stride = 1;
	for (auto axis = extents.size(); axis > 0; --axis)
	{
		strides[axis - 1] = stride;
		stride *= static_cast<std::ptrdiff_t>(extents[axis - 1]);
	}

	return strides;
}

std::vector<std::int32_t>
derive_core_space_axes(const mrc_header &header, std::size_t core_rank)
{
	REXLIB_ASSERT( core_rank == 3 || core_rank == 2 );

	std::vector<std::int32_t> axes;
	axes.reserve(core_rank);

	if (core_rank == 3)
	{
		axes.push_back(header.get_section_axis());
	}

	axes.push_back(header.get_row_axis());
	axes.push_back(header.get_column_axis());

	return axes;
}

std::vector<std::size_t> make_stored_order(std::size_t rank)
{
	std::vector<std::size_t> order(rank);
	std::iota(order.begin(), order.end(), std::size_t(0));

	return order;
}

// A file that states no axis correspondence at all is read as the file it
// would be if it named the three axes in order, which is what the writer that
// left the fields alone laid out.
std::vector<std::size_t> derive_axis_order(const mrc_header &header)
{
	const auto rank = derive_stored_extents(header).size();
	const auto core_rank = derive_core_rank(header);

	if (!has_axis_permutation(header))
	{
		if (!has_unset_axes(header))
		{
			throw image_format_error(
				"mrc_geometry: The axis correspondence of the file names "
				"anything but the three axes of space, one each."
			);
		}

		REXLIB_LOG_WARN(
			"An MRC file does not state which axes of space its columns, its "
			"rows and its sections run along. It is read as though they ran "
			"along 1, 2 and 3."
		);

		return make_stored_order(rank);
	}

	const auto core_axes = derive_core_space_axes(header, core_rank);
	const auto leading = rank - core_rank;

	auto order = make_stored_order(leading);
	for (auto space_axis = space_axis_count; space_axis > 0; --space_axis)
	{
		const auto stored = std::find(
			core_axes.cbegin(), core_axes.cend(), space_axis
		);
		if (stored != core_axes.cend())
		{
			order.push_back(
				leading +
				static_cast<std::size_t>(stored - core_axes.cbegin())
			);
		}
	}

	return order;
}

template <typename T>
std::vector<T> reorder(
	const std::vector<T> &values,
	const std::vector<std::size_t> &order
)
{
	std::vector<T> result;
	result.reserve(order.size());
	for (auto axis : order)
	{
		result.push_back(values[axis]);
	}

	return result;
}

void check_element_alignment(std::size_t offset, numerical_type data_type)
{
	const auto element_size = get_size(data_type);
	if (element_size == 0 || offset % element_size != 0)
	{
		throw image_format_error(
			"mrc_geometry: The values of the file do not begin at an offset "
			"their data type can be addressed at."
		);
	}
}

image_descriptor derive_descriptor(
	const mrc_header &header,
	const std::vector<std::size_t> &axis_order
)
{
	const auto data_type = mrc::get_data_type(header);
	check_element_alignment(mrc::get_data_offset(header), data_type);

	const auto extents = reorder(derive_stored_extents(header), axis_order);
	return image_descriptor(
		make_span(extents),
		derive_core_rank(header),
		data_type
	);
}

std::vector<std::ptrdiff_t> derive_strides(
	const mrc_header &header,
	const std::vector<std::size_t> &axis_order
)
{
	return reorder(
		derive_stored_strides(derive_stored_extents(header)),
		axis_order
	);
}

} // anonymous namespace

mrc_geometry::mrc_geometry(const mrc_header &header)
	: mrc_geometry(header, derive_axis_order(header))
{
}

mrc_geometry::mrc_geometry(
	const mrc_header &header,
	const std::vector<std::size_t> &axis_order
)
	: m_descriptor(derive_descriptor(header, axis_order))
	, m_strides(derive_strides(header, axis_order))
	, m_data_offset(mrc::get_data_offset(header))
{
}

const image_descriptor& mrc_geometry::get_descriptor() const noexcept
{
	return m_descriptor;
}

span<const std::ptrdiff_t> mrc_geometry::get_strides() const noexcept
{
	return make_span(m_strides.data(), m_strides.size());
}

std::size_t mrc_geometry::get_data_offset() const noexcept
{
	return m_data_offset;
}

std::size_t mrc_geometry::get_element_count() const noexcept
{
	const auto extents = m_descriptor.get_extents();
	return std::accumulate(
		extents.begin(),
		extents.end(),
		std::size_t(1),
		std::multiplies<std::size_t>()
	);
}

std::size_t mrc_geometry::get_data_size() const noexcept
{
	return get_element_count() * get_size(m_descriptor.get_data_type());
}

} // namespace mrc
} // namespace em
} // namespace rexlib
