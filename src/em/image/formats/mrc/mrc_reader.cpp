// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_reader.hpp"

#include "mrc_host_access.hpp"
#include "mrc_region_prefetch_plan.hpp"
#include "mrc_region_read_plan.hpp"
#include "mrc_region_transfer.hpp"

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <vector>

namespace rexlib
{
namespace em
{
namespace mrc
{

namespace
{

mrc_header read_header(const mrc_file_mapping &mapping)
{
	return parse_header(
		make_span(mapping.get_data(), mapping.get_size())
	);
}

void check_length(
	const mrc_file_mapping &mapping,
	const mrc_geometry &geometry
)
{
	const auto required =
		geometry.get_data_offset() + geometry.get_data_size();
	if (mapping.get_size() < required)
	{
		throw image_format_error(
			"mrc_reader: The file is shorter than the shape its header "
			"states."
		);
	}
}

} // anonymous namespace

mrc_reader::mrc_reader(const std::string &path)
	: m_mapping(path, read_only)
	, m_header(read_header(m_mapping))
	, m_geometry(m_header)
{
	check_length(m_mapping, m_geometry);
}

span<const std::size_t> mrc_reader::get_extents() const noexcept
{
	return m_geometry.get_extents();
}

std::size_t mrc_reader::get_core_rank() const noexcept
{
	return m_geometry.get_core_rank();
}

numerical_type mrc_reader::get_data_type() const noexcept
{
	return m_geometry.get_data_type();
}

const image_metadata& mrc_reader::get_metadata() const noexcept
{
	return m_metadata;
}

void mrc_reader::read(
	array_ref destination,
	const image_transfer_plan &regions
) const
{
	auto *array_data = get_host_data(destination);

	const auto &descriptor = destination.get_descriptor();
	const auto &layout = descriptor.get_layout();

	std::vector<std::size_t> array_extents;
	std::vector<std::ptrdiff_t> array_strides;
	layout.get_extents(array_extents);
	layout.get_strides(array_strides);

	// Validate the batch before the prefetch touches it.
	const mrc_region_read_plan plan(
		regions,
		m_geometry.get_extents(),
		m_geometry.get_strides(),
		make_span(array_extents),
		make_span(array_strides),
		layout.get_offset()
	);

	const mrc_region_prefetch_plan advice(
		regions,
		m_geometry,
		plan.get_offsets().get_file(),
		m_mapping.get_size(),
		make_prefetch_policy(compute_region_span(regions, m_geometry))
	);
	m_mapping.prefetch(advice.get_ranges());

	read_regions(
		plan,
		array_data,
		descriptor.get_data_type(),
		m_mapping.get_data() + m_geometry.get_data_offset(),
		m_geometry.get_data_type(),
		m_header.get_byte_order()
	);
}

} // namespace mrc
} // namespace em
} // namespace rexlib
