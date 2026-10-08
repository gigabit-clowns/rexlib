// SPDX-License-Identifier: GPL-3.0-only

#include "mapped_image_writer.hpp"

#include <em/image/formats/strided_transfer/image_region_transfer.hpp>
#include <em/image/formats/strided_transfer/image_region_write_walk.hpp>

#include <core/logger.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/ndarray/host_access.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <algorithm>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

namespace
{

image_file_mapping lay_out_file(
	const std::string &path,
	const image_file_layout &layout,
	span<const byte> preamble
)
{
	if (preamble.size() > layout.get_data_offset())
	{
		throw std::invalid_argument(
			"mapped_image_writer: The preamble reaches past where the values "
			"of the file begin."
		);
	}

	create_image_file(
		path,
		layout.get_data_offset() + layout.get_data_size()
	);

	image_file_mapping mapping(path, read_write);
	std::copy(preamble.begin(), preamble.end(), mapping.get_data());

	return mapping;
}

} // anonymous namespace

mapped_image_writer::mapped_image_writer(
	const std::string &path,
	image_file_layout layout,
	span<const byte> preamble
)
	: m_layout(std::move(layout))
	, m_mapping(lay_out_file(path, m_layout, preamble))
{
}

mapped_image_writer::~mapped_image_writer()
{
	try
	{
		m_mapping.flush();
	}
	catch (const std::exception &error)
	{
		REXLIB_LOG_ERROR(
			"Failed to flush an image file while closing it: {}",
			error.what()
		);
	}
	catch (...)
	{
		REXLIB_LOG_ERROR("Failed to flush an image file while closing it.");
	}
}

const image_descriptor& mapped_image_writer::get_descriptor() const noexcept
{
	return m_layout.get_descriptor();
}

void mapped_image_writer::write(
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

	const image_region_write_walk walk(
		regions,
		m_layout.get_descriptor().get_extents(),
		m_layout.get_strides(),
		make_span(array_extents),
		make_span(array_strides),
		layout.get_offset()
	);

	write_regions(
		walk,
		array_data,
		descriptor.get_data_type(),
		m_mapping.get_data() + m_layout.get_data_offset(),
		m_layout.get_descriptor().get_data_type(),
		m_layout.get_byte_order()
	);
}

void mapped_image_writer::flush()
{
	m_mapping.flush();
}

} // namespace em
} // namespace rexlib
