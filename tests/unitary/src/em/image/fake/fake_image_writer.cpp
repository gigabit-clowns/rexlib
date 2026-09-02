// SPDX-License-Identifier: GPL-3.0-only

#include "fake_image_writer.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace rexlib
{
namespace em
{

static std::size_t compute_element_count(span<const std::size_t> extents)
{
	return std::accumulate(
		extents.begin(),
		extents.end(),
		std::size_t(1),
		[] (std::size_t lhs, std::size_t rhs) { return lhs * rhs; }
	);
}

fake_image_writer::fake_image_writer(span<const std::size_t> extents)
	: m_extents(extents.begin(), extents.end())
	, m_data(compute_element_count(extents), 0)
	, m_flush_count(0)
{
}

fake_image_writer::~fake_image_writer() = default;

span<const std::size_t> fake_image_writer::get_extents() const noexcept
{
	return make_span(m_extents.data(), m_extents.size());
}

numerical_type fake_image_writer::get_data_type() const noexcept
{
	return numerical_type::int16;
}

std::int16_t fake_image_writer::get_element(std::size_t index) const
{
	return m_data.at(index);
}

std::size_t fake_image_writer::get_flush_count() const noexcept
{
	return m_flush_count;
}

void fake_image_writer::flush()
{
	++m_flush_count;
}

void fake_image_writer::write(
	const_array_ref source,
	const image_transfer_plan &regions
)
{
	const auto file_rank = m_extents.size();
	if (regions.get_destination_rank() != file_rank)
	{
		throw std::invalid_argument(
			"fake_image_writer::write: The destination offsets do not have "
			"the rank of the file."
		);
	}

	const auto *storage = source.get_storage();
	if (!storage)
	{
		throw std::invalid_argument(
			"fake_image_writer::write: The source is not initialized."
		);
	}

	const auto &layout = source.get_descriptor().get_layout();
	const auto source_rank = layout.get_rank();
	if (regions.get_source_rank() != source_rank)
	{
		throw std::invalid_argument(
			"fake_image_writer::write: The source offsets do not have the "
			"rank of the source."
		);
	}

	std::vector<std::size_t> source_extents;
	std::vector<std::ptrdiff_t> strides;
	layout.get_extents(source_extents);
	layout.get_strides(strides);

	const auto data_type = source.get_descriptor().get_data_type();
	if (
		data_type != numerical_type::int16 &&
		data_type != numerical_type::float32
	) {
		throw invalid_operation_error(
			"fake_image_writer::write: The file can not be written from the "
			"source data type."
		);
	}

	const auto *base = static_cast<const byte*>(storage->get_host_ptr());
	if (!base)
	{
		throw invalid_operation_error(
			"fake_image_writer::write: The source is not host accessible."
		);
	}

	const auto extents = regions.get_extents();
	const auto rank = regions.get_rank();
	const auto source_leading = source_rank - rank;
	const auto destination_leading = file_rank - rank;
	const auto element_size = get_size(data_type);
	const auto count = compute_element_count(extents);

	std::vector<std::size_t> coordinates(rank, 0);

	for (std::size_t region = 0; region < regions.get_size(); ++region)
	{
		const auto source_offset = regions.get_source_offset(region);
		const auto destination_offset =
			regions.get_destination_offset(region);

		for (std::size_t i = 0; i < file_rank; ++i)
		{
			const auto extent = get_region_extent(regions, file_rank, i);
			if (destination_offset[i] + extent > m_extents[i])
			{
				throw std::out_of_range(
					"fake_image_writer::write: The region is not contained "
					"in the file."
				);
			}
		}

		for (std::size_t i = 0; i < source_rank; ++i)
		{
			const auto extent = get_region_extent(regions, source_rank, i);
			if (source_offset[i] + extent > source_extents[i])
			{
				throw std::out_of_range(
					"fake_image_writer::write: The region is not contained "
					"in the source."
				);
			}
		}

		std::fill(coordinates.begin(), coordinates.end(), std::size_t(0));
		auto remaining = count;
		while (remaining-- > 0)
		{
			std::size_t target = 0;
			for (std::size_t i = 0; i < file_rank; ++i)
			{
				const auto c = i < destination_leading
					? std::size_t(0)
					: coordinates[i - destination_leading];
				target = (target * m_extents[i]) + destination_offset[i] + c;
			}

			auto position = layout.get_offset();
			for (std::size_t i = 0; i < source_rank; ++i)
			{
				const auto c = i < source_leading
					? std::size_t(0)
					: coordinates[i - source_leading];
				position += static_cast<std::ptrdiff_t>(
					source_offset[i] + c
				) * strides[i];
			}

			const auto *element = base +
				(position * static_cast<std::ptrdiff_t>(element_size));
			if (data_type == numerical_type::int16)
			{
				m_data[target] = *reinterpret_cast<const std::int16_t*>(
					element
				);
			}
			else
			{
				m_data[target] = static_cast<std::int16_t>(
					*reinterpret_cast<const float*>(element)
				);
			}

			for (auto i = rank; i-- > 0; )
			{
				if (++coordinates[i] < extents[i])
				{
					break;
				}
				coordinates[i] = 0;
			}
		}
	}
}

} // namespace em
} // namespace rexlib
