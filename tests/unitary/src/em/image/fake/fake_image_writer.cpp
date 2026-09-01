// SPDX-License-Identifier: GPL-3.0-only

#include "fake_image_writer.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
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
	, m_descriptor(
		strided_layout::make_contiguous_layout(extents),
		numerical_type::int16
	)
	, m_flush_count(0)
{
}

fake_image_writer::~fake_image_writer() = default;

const array_descriptor& fake_image_writer::get_descriptor() const noexcept
{
	return m_descriptor;
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
	const image_region_list &regions
)
{
	const auto rank = m_extents.size();
	if (regions.get_dataset_rank() != rank)
	{
		throw std::invalid_argument(
			"fake_image_writer::write: The dataset offsets do not have the "
			"rank of the dataset."
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
	if (regions.get_array_rank() != layout.get_rank())
	{
		throw std::invalid_argument(
			"fake_image_writer::write: The extents do not have the rank of "
			"the source."
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
			"fake_image_writer::write: The dataset can not be written from "
			"the source data type."
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
	const auto array_rank = regions.get_array_rank();
	const auto leading = array_rank - rank;
	const auto element_size = get_size(data_type);
	const auto count = compute_element_count(extents);

	std::vector<std::size_t> coordinates(array_rank, 0);

	for (std::size_t region = 0; region < regions.get_size(); ++region)
	{
		const auto dataset_offset = regions.get_dataset_offset(region);
		const auto array_offset = regions.get_array_offset(region);

		for (std::size_t i = 0; i < rank; ++i)
		{
			if (dataset_offset[i] + extents[leading + i] > m_extents[i])
			{
				throw std::out_of_range(
					"fake_image_writer::write: The region is not contained "
					"in the dataset."
				);
			}
		}

		for (std::size_t i = 0; i < array_rank; ++i)
		{
			if (array_offset[i] + extents[i] > source_extents[i])
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
			auto position = layout.get_offset();
			for (std::size_t i = 0; i < array_rank; ++i)
			{
				position += static_cast<std::ptrdiff_t>(
					array_offset[i] + coordinates[i]
				) * strides[i];
			}
			for (std::size_t i = 0; i < rank; ++i)
			{
				target = (target * m_extents[i]) +
					dataset_offset[i] + coordinates[leading + i];
			}

			const auto *element = base + (position * static_cast<std::ptrdiff_t>(
				element_size
			));
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

			for (auto i = array_rank; i-- > 0; )
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
