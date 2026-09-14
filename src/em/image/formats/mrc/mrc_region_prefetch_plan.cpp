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

mrc_prefetch_policy::mrc_prefetch_policy(
	std::size_t gap_tolerance,
	std::size_t byte_budget,
	std::size_t page_size
) noexcept
	: m_gap_tolerance(gap_tolerance)
	, m_byte_budget(byte_budget)
	, m_page_size(page_size)
{
}

std::size_t mrc_prefetch_policy::get_gap_tolerance() const noexcept
{
	return m_gap_tolerance;
}

std::size_t mrc_prefetch_policy::get_byte_budget() const noexcept
{
	return m_byte_budget;
}

std::size_t mrc_prefetch_policy::get_page_size() const noexcept
{
	return m_page_size;
}

mrc_prefetch_policy make_prefetch_policy(std::size_t region_span) noexcept
{
	const auto page_size = get_page_size();
	const auto floor = std::max(region_span, page_size);

	return mrc_prefetch_policy(
		std::min(floor, default_prefetch_gap_cap),
		default_prefetch_budget,
		page_size
	);
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

namespace
{

// The stretch one region occupies: grown back to the page its start falls in,
// and cut short where the mapping ends.
memory_range locate_region(
	std::size_t start,
	std::size_t region_span,
	std::size_t mapped_size,
	std::size_t page_size
) noexcept
{
	const auto first = start - (start % page_size);
	const auto last = std::min(start + region_span, mapped_size);

	return memory_range(first, last - first);
}

memory_range merge(
	const memory_range &previous,
	const memory_range &next
) noexcept
{
	const auto end = std::max(
		previous.get_offset() + previous.get_size(),
		next.get_offset() + next.get_size()
	);

	return memory_range(previous.get_offset(), end - previous.get_offset());
}

// Whether a stretch is asked for together with the one before it: it starts
// within the tolerance of where that one ends, and taking it in does not grow
// that one past the budget of a step. One already past the budget, a single
// region wider than it, still takes in what lies within it.
bool joins(
	const memory_range &previous,
	const memory_range &next,
	const mrc_prefetch_policy &policy
) noexcept
{
	const auto end = previous.get_offset() + previous.get_size();
	if (next.get_offset() > end + policy.get_gap_tolerance())
	{
		return false;
	}

	const auto merged_size = merge(previous, next).get_size();

	return merged_size <=
		std::max(policy.get_byte_budget(), previous.get_size());
}

} // anonymous namespace

mrc_region_prefetch_plan::mrc_region_prefetch_plan(
	const image_transfer_plan &regions,
	const mrc_geometry &geometry,
	span<const std::ptrdiff_t> file_offsets,
	std::size_t mapped_size,
	const mrc_prefetch_policy &policy
)
{
	REXLIB_ASSERT(policy.get_page_size() > 0);
	REXLIB_ASSERT(
		std::is_sorted(file_offsets.begin(), file_offsets.end())
	);

	m_step_first_range.push_back(0);
	m_step_first_region.push_back(0);

	if (file_offsets.empty())
	{
		return;
	}

	const auto regions_per_range = gather_ranges(
		regions, geometry, file_offsets, mapped_size, policy
	);

	gather_steps(
		make_span(regions_per_range),
		file_offsets.size(),
		policy.get_byte_budget()
	);
}

std::vector<std::size_t> mrc_region_prefetch_plan::gather_ranges(
	const image_transfer_plan &regions,
	const mrc_geometry &geometry,
	span<const std::ptrdiff_t> file_offsets,
	std::size_t mapped_size,
	const mrc_prefetch_policy &policy
)
{
	std::vector<std::size_t> regions_per_range;

	const auto region_span = compute_region_span(regions, geometry);
	if (region_span == 0)
	{
		return regions_per_range;
	}

	const auto element_size = get_size(geometry.get_data_type());
	const auto data_offset = geometry.get_data_offset();

	for (const auto offset : file_offsets)
	{
		REXLIB_ASSERT(offset >= 0);
		const auto start = data_offset +
			static_cast<std::size_t>(offset) * element_size;
		if (start >= mapped_size)
		{
			continue;
		}

		const auto stretch = locate_region(
			start, region_span, mapped_size, policy.get_page_size()
		);

		if (!m_ranges.empty() && joins(m_ranges.back(), stretch, policy))
		{
			m_ranges.back() = merge(m_ranges.back(), stretch);
			++regions_per_range.back();
		}
		else
		{
			m_ranges.push_back(stretch);
			regions_per_range.push_back(1);
		}
	}

	return regions_per_range;
}

void mrc_region_prefetch_plan::gather_steps(
	span<const std::size_t> regions_per_range,
	std::size_t region_count,
	std::size_t byte_budget
)
{
	std::size_t step_bytes = 0;
	std::size_t regions_seen = 0;
	for (std::size_t i = 0; i < m_ranges.size(); ++i)
	{
		const auto size = m_ranges[i].get_size();
		const auto opens_step = i == m_step_first_range.back();
		if (!opens_step && step_bytes + size > byte_budget)
		{
			m_step_first_range.push_back(i);
			m_step_first_region.push_back(regions_seen);
			step_bytes = 0;
		}

		step_bytes += size;
		regions_seen += regions_per_range[i];
	}

	// The last step takes every region the stretches left out, which is what
	// makes the steps tile the batch however little there was to ask for.
	m_step_first_range.push_back(m_ranges.size());
	m_step_first_region.push_back(region_count);
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
