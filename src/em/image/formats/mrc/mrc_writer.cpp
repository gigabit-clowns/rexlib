// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_writer.hpp"

#include "mrc_constants.hpp"
#include "mrc_host_access.hpp"
#include "mrc_mode.hpp"
#include "mrc_region_transfer.hpp"
#include "mrc_region_write_plan.hpp"

#include <core/logger.hpp>
#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/library_version.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <sstream>
#include <stdexcept>
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

std::string make_signature()
{
	std::ostringstream text;
	text << "Created by rexlib " << get_library_version();
	return text.str();
}

void check_stack_extent(std::size_t extent, const char *what)
{
	if (extent == 1)
	{
		std::ostringstream message;
		message << "mrc::make_header: The MRC format holds no " << what
			<< ": it states it as the shape without that axis.";
		throw invalid_operation_error(message.str());
	}
}

mrc_file_mapping lay_out_file(
	const std::string &path,
	const mrc_geometry &geometry
)
{
	create_file(
		path,
		geometry.get_data_offset() + geometry.get_data_size()
	);

	return mrc_file_mapping(path, read_write);
}

} // anonymous namespace

mrc_writer::mrc_writer(
	const std::string &path,
	span<const std::size_t> extents,
	std::size_t core_rank,
	numerical_type data_type
)
	: m_header(make_header(extents, core_rank, data_type))
	, m_geometry(m_header)
	, m_mapping(lay_out_file(path, m_geometry))
{
	serialize_header(
		m_header,
		make_span(m_mapping.get_data(), m_mapping.get_size())
	);
}

mrc_writer::~mrc_writer()
{
	// A destructor can not report a failure, so a writer whose failures
	// matter is flushed explicitly beforehand. This is what keeps a writer
	// that was not from leaving the header unwritten, and saying so is all
	// that can be done about a file that never reached the storage.
	try
	{
		m_mapping.flush();
	}
	catch (const std::exception &error)
	{
		REXLIB_LOG_ERROR(
			"Failed to flush an MRC file while closing it: {}", error.what()
		);
	}
	catch (...)
	{
		REXLIB_LOG_ERROR("Failed to flush an MRC file while closing it.");
	}
}

span<const std::size_t> mrc_writer::get_extents() const noexcept
{
	return m_geometry.get_extents();
}

std::size_t mrc_writer::get_core_rank() const noexcept
{
	return m_geometry.get_core_rank();
}

numerical_type mrc_writer::get_data_type() const noexcept
{
	return m_geometry.get_data_type();
}

void mrc_writer::write(
	const_array_ref source,
	const image_transfer_plan &regions
)
{
	const auto *array_data = get_host_data(source);

	const auto &descriptor = source.get_descriptor();
	const auto &layout = descriptor.get_layout();

	std::vector<std::size_t> array_extents;
	std::vector<std::ptrdiff_t> array_strides;
	layout.get_extents(array_extents);
	layout.get_strides(array_strides);

	const mrc_region_write_plan plan(
		regions,
		m_geometry.get_extents(),
		m_geometry.get_strides(),
		make_span(array_extents),
		make_span(array_strides),
		layout.get_offset()
	);

	write_regions(
		plan,
		array_data,
		descriptor.get_data_type(),
		m_mapping.get_data() + m_geometry.get_data_offset(),
		m_geometry.get_data_type(),
		m_header.get_byte_order()
	);
}

void mrc_writer::flush()
{
	m_mapping.flush();
}

mrc_header make_header(
	span<const std::size_t> extents,
	std::size_t core_rank,
	numerical_type data_type
)
{
	const auto rank = extents.size();
	if (core_rank == 0 || core_rank > rank)
	{
		throw std::invalid_argument(
			"mrc::make_header: The core rank must name at least one and at "
			"most every extent."
		);
	}

	if (rank < 2 || rank > 4)
	{
		throw invalid_operation_error(
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
		check_stack_extent(extents[0], "stack of one image");

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
		check_stack_extent(extents[0], "stack of one volume");
		check_stack_extent(extents[1], "stack of volumes one section thick");

		const auto depth = static_cast<std::int32_t>(extents[1]);
		header.set_section_count(
			static_cast<std::int32_t>(extents[0] * extents[1]));
		header.set_section_sampling(depth);
		header.set_space_group(first_volume_stack_space_group);
	}
	else
	{
		throw invalid_operation_error(
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
