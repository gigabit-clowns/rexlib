// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_region_prefetch_plan.hpp"

#include "mrc_geometry.hpp"

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/core/system/host.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <algorithm>

namespace rexlib
{
namespace em
{
namespace mrc
{

mrc_prefetch_policy make_prefetch_policy(std::size_t region_span) noexcept
{
	const auto page_size = get_page_size();
	const auto floor = std::max(region_span, page_size);

	const mrc_prefetch_policy policy = {
		std::min(floor, default_prefetch_gap_cap),
		default_prefetch_budget,
		page_size
	};

	return policy;
}

std::size_t compute_region_span(
	const image_transfer_plan &regions,
	const mrc_geometry &geometry
) noexcept
{
	const auto strides = geometry.get_strides();
	const auto rank = strides.size();

	std::size_t elements = 1;
	for (std::size_t axis = 0; axis < rank; ++axis)
	{
		const auto extent = get_region_extent(regions, rank, axis);
		if (extent == 0)
		{
			return 0;
		}

		REXLIB_ASSERT(strides[axis] >= 0);
		elements +=
			(extent - 1) * static_cast<std::size_t>(strides[axis]);
	}

	return elements * get_size(geometry.get_data_type());
}

mrc_region_prefetch_plan::mrc_region_prefetch_plan(
	const image_transfer_plan &regions,
	const mrc_geometry &geometry,
	span<const std::ptrdiff_t> file_offsets,
	std::size_t mapped_size,
	const mrc_prefetch_policy &policy
)
{
	REXLIB_ASSERT(policy.page_size > 0);
	REXLIB_ASSERT(
		std::is_sorted(file_offsets.begin(), file_offsets.end())
	);

	m_step_first_range.push_back(0);
	m_step_first_region.push_back(0);

	if (file_offsets.empty())
	{
		return;
	}

	const auto region_span = compute_region_span(regions, geometry);
	const auto element_size = get_size(geometry.get_data_type());
	const auto data_offset = geometry.get_data_offset();

	std::vector<std::size_t> regions_per_range;
	for (const auto offset : file_offsets)
	{
		if (region_span == 0)
		{
			break;
		}

		REXLIB_ASSERT(offset >= 0);
		const auto start = data_offset +
			static_cast<std::size_t>(offset) * element_size;
		if (start >= mapped_size)
		{
			continue;
		}

		const auto first = start - (start % policy.page_size);
		const auto last = std::min(start + region_span, mapped_size);

		if (!m_ranges.empty())
		{
			auto &previous = m_ranges.back();
			const auto end = previous.get_offset() + previous.get_size();
			if (first <= end + policy.gap_tolerance)
			{
				previous = memory_range(
					previous.get_offset(),
					std::max(end, last) - previous.get_offset()
				);
				++regions_per_range.back();
				continue;
			}
		}

		const memory_range range(first, last - first);
		m_ranges.push_back(range);
		regions_per_range.push_back(1);
	}

	// The steps tile the batch whether or not there is anything to advise,
	// so that walking them walks every region exactly once.
	if (m_ranges.empty())
	{
		m_step_first_range.push_back(0);
		m_step_first_region.push_back(file_offsets.size());
		return;
	}

	std::size_t step_bytes = 0;
	std::size_t regions_seen = 0;
	for (std::size_t i = 0; i < m_ranges.size(); ++i)
	{
		const auto size = m_ranges[i].get_size();
		const auto opens_step = i == m_step_first_range.back();
		if (!opens_step && step_bytes + size > policy.byte_budget)
		{
			m_step_first_range.push_back(i);
			m_step_first_region.push_back(regions_seen);
			step_bytes = 0;
		}

		step_bytes += size;
		regions_seen += regions_per_range[i];
	}

	// The last step takes whatever regions were left out of the stretches,
	// which are the ones there was nothing to ask for.
	m_step_first_range.push_back(m_ranges.size());
	m_step_first_region.push_back(file_offsets.size());
}

std::size_t mrc_region_prefetch_plan::get_step_count() const noexcept
{
	return m_step_first_range.size() - 1;
}

span<const memory_range>
mrc_region_prefetch_plan::get_ranges() const noexcept
{
	return make_span(m_ranges.data(), m_ranges.size());
}

span<const memory_range>
mrc_region_prefetch_plan::get_step_ranges(std::size_t step) const noexcept
{
	REXLIB_ASSERT(step < get_step_count());

	const auto first = m_step_first_range[step];

	return make_span(
		m_ranges.data() + first,
		m_step_first_range[step + 1] - first
	);
}

std::size_t
mrc_region_prefetch_plan::get_step_first_region(
	std::size_t step
) const noexcept
{
	REXLIB_ASSERT(step < get_step_count());

	return m_step_first_region[step];
}

std::size_t
mrc_region_prefetch_plan::get_step_region_count(
	std::size_t step
) const noexcept
{
	REXLIB_ASSERT(step < get_step_count());

	return m_step_first_region[step + 1] - m_step_first_region[step];
}

} // namespace mrc
} // namespace em
} // namespace rexlib
