// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_geometry.hpp"

#include "mrc_constants.hpp"
#include "mrc_mode.hpp"

#include <core/logger.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/library_version.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include <algorithm>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

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

// MRC2014 states a stack of volumes by its space group alone, dividing the
// sections into volumes of the sampling along them, however many or few of
// either there are. Only the image space group leaves a case open, one
// section, which the caller settles.
file_kind derive_file_kind(
	const mrc_header &header,
	mrc_single_section single_section
) noexcept
{
	const auto space_group = header.get_space_group();

	if (is_volume_stack_space_group(space_group))
	{
		return file_kind::volume_stack;
	}

	if (space_group != image_stack_space_group)
	{
		return file_kind::volume;
	}

	const auto single = header.get_section_count() == 1 &&
		single_section == mrc_single_section::image;
	return single ? file_kind::image : file_kind::image_stack;
}

std::vector<std::size_t> derive_stored_extents(
	const mrc_header &header,
	mrc_single_section single_section
)
{
	const auto kind = derive_file_kind(header, single_section);
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

std::size_t derive_core_rank(
	const mrc_header &header,
	mrc_single_section single_section
) noexcept
{
	switch (derive_file_kind(header, single_section))
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
std::vector<std::size_t> derive_axis_order(
	const mrc_header &header,
	mrc_single_section single_section
)
{
	const auto rank = derive_stored_extents(header, single_section).size();
	const auto core_rank = derive_core_rank(header, single_section);

	if (!has_axis_permutation(header))
	{
		if (!has_unset_axes(header))
		{
			throw image_format_error(
				"mrc::derive_file_layout: The axis correspondence of the file "
				"names anything but the three axes of space, one each."
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

image_descriptor derive_descriptor(
	const mrc_header &header,
	mrc_single_section single_section,
	const std::vector<std::size_t> &axis_order
)
{
	const auto extents =
		reorder(derive_stored_extents(header, single_section), axis_order);
	return image_descriptor(
		make_span(extents),
		derive_core_rank(header, single_section),
		mrc::get_data_type(header)
	);
}

std::vector<std::ptrdiff_t> derive_strides(
	const mrc_header &header,
	mrc_single_section single_section,
	const std::vector<std::size_t> &axis_order
)
{
	return reorder(
		derive_stored_strides(derive_stored_extents(header, single_section)),
		axis_order
	);
}

std::string make_signature()
{
	std::ostringstream text;
	text << "Created by rexlib " << get_library_version();
	return text.str();
}

} // anonymous namespace

image_file_layout derive_file_layout(
	const mrc_header &header,
	mrc_single_section single_section
)
{
	const auto axis_order = derive_axis_order(header, single_section);

	return image_file_layout(
		derive_descriptor(header, single_section, axis_order),
		derive_strides(header, single_section, axis_order),
		mrc::get_data_offset(header),
		header.get_byte_order()
	);
}

mrc_header make_header(const image_descriptor &descriptor)
{
	const auto extents = descriptor.get_extents();
	const auto core_rank = descriptor.get_core_rank();
	const auto data_type = descriptor.get_data_type();
	const auto rank = extents.size();

	if (rank < 2 || rank > 4)
	{
		throw unsupported_operation_error(
			"mrc::make_header: The MRC format holds no file of that rank."
		);
	}

	mrc_header header;
	header.set_mode(get_mode(data_type));
	if (needs_imod_unsigned_flag(data_type))
	{
		header.set_imod_stamp(imod_stamp_value);
	}

	const auto columns = static_cast<std::int32_t>(extents[rank - 1]);
	const auto rows = static_cast<std::int32_t>(extents[rank - 2]);
	header.set_column_count(columns);
	header.set_row_count(rows);
	header.set_column_sampling(columns);
	header.set_row_sampling(rows);

	if (rank == 2 && core_rank == 2)
	{
		header.set_section_count(1);
		header.set_section_sampling(1);
		header.set_space_group(image_stack_space_group);
	}
	else if (rank == 3 && core_rank == 2)
	{
		header.set_section_count(static_cast<std::int32_t>(extents[0]));
		header.set_section_sampling(1);
		header.set_space_group(image_stack_space_group);
	}
	else if (rank == 3 && core_rank == 3)
	{
		const auto sections = static_cast<std::int32_t>(extents[0]);
		header.set_section_count(sections);
		header.set_section_sampling(sections);
		header.set_space_group(volume_space_group);
	}
	else if (rank == 4 && core_rank == 3)
	{
		const auto depth = static_cast<std::int32_t>(extents[1]);
		header.set_section_count(
			static_cast<std::int32_t>(extents[0] * extents[1]));
		header.set_section_sampling(depth);
		header.set_space_group(first_volume_stack_space_group);
	}
	else
	{
		throw unsupported_operation_error(
			"mrc::make_header: The MRC format holds no file of that rank "
			"and core rank."
		);
	}

	header.set_cell_size({{
		static_cast<float>(header.get_column_count()),
		static_cast<float>(header.get_row_count()),
		static_cast<float>(header.get_section_sampling())
	}});
	header.set_cell_angles({{90.0F, 90.0F, 90.0F}});
	header.set_column_axis(1);
	header.set_row_axis(2);
	header.set_section_axis(3);
	header.set_version(written_version);

	// The sentinels the format reserves for statistics that were not
	// computed: a minimum above the maximum, a mean below both, and a
	// negative deviation.
	header.set_data_min(0.0F);
	header.set_data_max(-1.0F);
	header.set_data_mean(-2.0F);
	header.set_data_rms(-1.0F);

	// Sign the header
	header.add_label(make_signature());

	return header;
}

} // namespace mrc
} // namespace em
} // namespace rexlib
