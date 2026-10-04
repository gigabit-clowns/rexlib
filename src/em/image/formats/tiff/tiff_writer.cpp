// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_writer.hpp"

#include "tiff_page_regions.hpp"
#include "tiff_sample_type.hpp"

#include <em/image/formats/strided_transfer/image_host_access.hpp>
#include <em/image/formats/strided_transfer/image_region_offsets.hpp>
#include <em/image/formats/strided_transfer/image_region_transfer.hpp>
#include <em/image/formats/strided_transfer/image_region_write_walk.hpp>

#include <core/logger.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/memory/byte_order.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{
namespace tiff
{

namespace
{

const std::size_t page_rank = 2;
const std::size_t stack_rank = 3;

// Half of what a classic file addresses. The other half is left to the
// directories and to LZW, which makes samples it can not compress larger.
const std::size_t classic_sample_budget = std::size_t(1) << 31;

// How many bytes of samples a strip holds at most, unless one row exceeds
// them.
const std::size_t strip_size = std::size_t(1) << 18;

const image_descriptor& checked(const image_descriptor &descriptor)
{
	const auto extents = descriptor.get_extents();
	const auto rank = extents.size();
	if ((rank != page_rank && rank != stack_rank) ||
		descriptor.get_core_rank() != page_rank)
	{
		throw std::invalid_argument(
			"tiff_writer: A TIFF file holds an image or a stack of images."
		);
	}

	if (std::find(extents.begin(), extents.end(), 0) != extents.end())
	{
		throw std::invalid_argument(
			"tiff_writer: A TIFF file has no extent of zero."
		);
	}

	if (!is_supported(descriptor.get_data_type()))
	{
		throw unsupported_operation_error(
			"tiff_writer: The TIFF format has no sample this format "
			"transfers for this data type."
		);
	}

	return descriptor;
}

std::size_t get_page_count(const image_descriptor &descriptor) noexcept
{
	const auto extents = descriptor.get_extents();
	return extents.size() == stack_rank ? extents[0] : 1;
}

std::size_t get_page_size(const image_descriptor &descriptor) noexcept
{
	const auto extents = get_core_extents(descriptor);
	return extents[0] * extents[1] * get_size(descriptor.get_data_type());
}

tiff_file_mode get_file_mode(const image_descriptor &descriptor) noexcept
{
	// Compared by division so that a file too large to count does not wrap.
	const auto fits = get_page_count(descriptor) <=
		classic_sample_budget / get_page_size(descriptor);

	return fits ? tiff_file_mode::create : tiff_file_mode::create_big;
}

tiff_page_layout make_page_layout(const image_descriptor &descriptor)
{
	const auto extents = get_core_extents(descriptor);
	const auto data_type = descriptor.get_data_type();
	const auto row_size = extents[1] * get_size(data_type);

	return tiff_page_layout::make_striped(
		extents[1],
		extents[0],
		data_type,
		std::max<std::size_t>(strip_size / row_size, 1)
	);
}

tiff_compression get_compression(numerical_type data_type) noexcept
{
	switch (data_type)
	{
	case numerical_type::float16:
	case numerical_type::float32:
		return tiff_compression::none;
	default:
		return tiff_compression::lzw;
	}
}

std::vector<std::ptrdiff_t>
make_contiguous_strides(span<const std::size_t> extents)
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

bool covers_whole_pages(
	const image_transfer_shape &shape,
	span<const std::size_t> page_extents
) noexcept
{
	const auto extents = shape.get_extents();
	return
		extents.size() >= page_rank &&
		std::equal(
			page_extents.begin(),
			page_extents.end(),
			extents.end() - page_rank
		);
}

} // anonymous namespace

tiff_writer::tiff_writer(
	const std::string &path,
	image_descriptor descriptor
)
	: m_descriptor(std::move(descriptor))
	, m_mutex()
	, m_file(path, get_file_mode(checked(m_descriptor)))
	, m_page_count(get_page_count(m_descriptor))
	, m_next_page(0)
	, m_page(get_page_size(m_descriptor))
{
}

tiff_writer::~tiff_writer()
{
	if (m_next_page < m_page_count)
	{
		REXLIB_LOG_WARN(
			"A TIFF file is closed with {} of its {} pages written.",
			m_next_page,
			m_page_count
		);
	}
}

const image_descriptor& tiff_writer::get_descriptor() const noexcept
{
	return m_descriptor;
}

void tiff_writer::write(
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

	// The regions are resolved against the whole file first, and so bounds
	// checked, before any page is written.
	const auto file_extents = m_descriptor.get_extents();
	const auto file_strides = make_contiguous_strides(file_extents);
	const image_region_offsets resolved(
		regions,
		file_extents,
		make_span(file_strides.data(), file_strides.size()),
		make_span(array_extents),
		make_span(array_strides),
		layout.get_offset()
	);
	if (resolved.get_region_count() == 0)
	{
		return;
	}

	// A region with the extents of a page that is contained in the file
	// can only begin at the first row and column of one.
	const auto page_extents = get_core_extents(m_descriptor);
	if (!covers_whole_pages(regions.get_shape(), page_extents))
	{
		throw unsupported_operation_error(
			"tiff_writer: A TIFF file is written whole pages at a time, "
			"and a region covers part of one."
		);
	}

	const tiff_page_regions pages(regions);
	const auto page_count = pages.get_page_count();
	const auto page_layout = make_page_layout(m_descriptor);
	const auto compression = get_compression(m_descriptor.get_data_type());
	const std::array<std::ptrdiff_t, 2> page_strides = {{
		static_cast<std::ptrdiff_t>(page_extents[1]),
		1
	}};

	const std::lock_guard<std::mutex> lock(m_mutex);

	for (std::size_t position = 0; position < page_count; ++position)
	{
		if (pages.get_page(position) != m_next_page + position ||
			pages.get_regions(position).get_region_count() != 1)
		{
			throw unsupported_operation_error(
				"tiff_writer: The pages of a TIFF file are written in "
				"order and once, and the regions do not cover a run of "
				"consecutive pages beginning at page " +
				std::to_string(m_next_page) + ", the first one not yet "
				"written."
			);
		}
	}

	for (std::size_t position = 0; position < page_count; ++position)
	{
		const image_region_write_walk walk(
			pages.get_regions(position),
			page_extents,
			make_span(page_strides),
			make_span(array_extents),
			make_span(array_strides),
			layout.get_offset()
		);

		write_regions(
			walk,
			array_data,
			descriptor.get_data_type(),
			m_page.data(),
			m_descriptor.get_data_type(),
			get_system_byte_order()
		);

		m_file.write_page(
			page_layout,
			compression,
			make_span(m_page.data(), m_page.size())
		);
		++m_next_page;
	}
}

void tiff_writer::flush()
{
}

} // namespace tiff
} // namespace em
} // namespace rexlib
