// SPDX-License-Identifier: GPL-3.0-only

#include "fake_image_reader.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>

#include <numeric>
#include <stdexcept>
#include <utility>

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

fake_image_reader::fake_image_reader(
	span<const std::size_t> extents,
	image_access_traits traits,
	image_metadata metadata
)
	: m_extents(extents.begin(), extents.end())
	, m_descriptor(
		strided_layout::make_contiguous_layout(extents),
		numerical_type::int16
	)
	, m_traits(std::move(traits))
	, m_metadata(std::move(metadata))
{
	const auto count = compute_element_count(extents);
	m_data.reserve(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		m_data.push_back(static_cast<std::int16_t>(i));
	}
}

fake_image_reader::~fake_image_reader() = default;

const array_descriptor& fake_image_reader::get_descriptor() const noexcept
{
	return m_descriptor;
}

const image_metadata& fake_image_reader::get_metadata() const noexcept
{
	return m_metadata;
}

const image_access_traits&
fake_image_reader::get_access_traits() const noexcept
{
	return m_traits;
}

std::int16_t fake_image_reader::get_element(std::size_t index) const
{
	return m_data.at(index);
}

void fake_image_reader::read(
	array_ref destination,
	const image_region_batch &regions
) const
{
	const auto rank = m_extents.size();
	if (regions.get_file_rank() != rank)
	{
		throw std::invalid_argument(
			"fake_image_reader::read: The file offsets do not have the "
			"rank of the file."
		);
	}

	const auto *storage = destination.get_storage();
	if (!storage)
	{
		throw std::invalid_argument(
			"fake_image_reader::read: The destination is not initialized."
		);
	}

	// Everything below this point is read once for the whole batch. That is
	// the reason the regions arrive together.
	const auto &layout = destination.get_descriptor().get_layout();
	if (regions.get_array_rank() != layout.get_rank())
	{
		throw std::invalid_argument(
			"fake_image_reader::read: The extents do not have the rank of "
			"the destination."
		);
	}

	std::vector<std::size_t> destination_extents;
	std::vector<std::ptrdiff_t> strides;
	layout.get_extents(destination_extents);
	layout.get_strides(strides);

	const auto data_type = destination.get_descriptor().get_data_type();
	if (
		data_type != numerical_type::int16 &&
		data_type != numerical_type::float32
	) {
		throw invalid_operation_error(
			"fake_image_reader::read: The destination data type can not be "
			"produced from the file."
		);
	}

	auto *base = static_cast<byte*>(destination.get_storage()->get_host_ptr());
	if (!base)
	{
		throw invalid_operation_error(
			"fake_image_reader::read: The destination is not host accessible."
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
		const auto file_offset = regions.get_file_offset(region);
		const auto array_offset = regions.get_array_offset(region);

		for (std::size_t i = 0; i < rank; ++i)
		{
			if (file_offset[i] + extents[leading + i] > m_extents[i])
			{
				throw std::out_of_range(
					"fake_image_reader::read: The region is not contained in "
					"the file."
				);
			}
		}

		for (std::size_t i = 0; i < array_rank; ++i)
		{
			if (array_offset[i] + extents[i] > destination_extents[i])
			{
				throw std::out_of_range(
					"fake_image_reader::read: The region does not fit in the "
					"destination."
				);
			}
		}

		std::fill(coordinates.begin(), coordinates.end(), std::size_t(0));
		auto remaining = count;
		while (remaining-- > 0)
		{
			std::size_t source = 0;
			auto position = layout.get_offset();
			for (std::size_t i = 0; i < array_rank; ++i)
			{
				position += static_cast<std::ptrdiff_t>(
					array_offset[i] + coordinates[i]
				) * strides[i];
			}
			for (std::size_t i = 0; i < rank; ++i)
			{
				source = (source * m_extents[i]) +
					file_offset[i] + coordinates[leading + i];
			}

			const auto value = m_data[source];
			auto *target = base + (position * static_cast<std::ptrdiff_t>(
				element_size
			));
			if (data_type == numerical_type::int16)
			{
				*reinterpret_cast<std::int16_t*>(target) = value;
			}
			else
			{
				*reinterpret_cast<float*>(target) = static_cast<float>(value);
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
