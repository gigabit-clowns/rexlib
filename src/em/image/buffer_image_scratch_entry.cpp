// SPDX-License-Identifier: GPL-3.0-only

#include "buffer_image_scratch_entry.hpp"

#include <em/image/formats/strided_transfer/image_host_access.hpp>
#include <em/image/formats/strided_transfer/image_region_copy.hpp>

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <stdexcept>
#include <utility>
#include <vector>

namespace rexlib
{
namespace em
{

buffer_image_scratch_entry::buffer_image_scratch_entry(
	image_scratch_slots slots,
	array values,
	std::size_t run_length
)
	: m_slots(std::move(slots))
	, m_runs(m_slots.get_count(), run_length)
	, m_values(std::move(values))
{
	get_host_data(const_array_ref(m_values));

	std::vector<std::size_t> extents;
	m_values.get_descriptor().get_layout().get_extents(extents);
	if (extents.empty() || extents.front() != m_slots.get_count())
	{
		throw std::invalid_argument(
			"buffer_image_scratch_entry: The first extent of the values is "
			"not the number of slots."
		);
	}
}

buffer_image_scratch_entry::~buffer_image_scratch_entry() = default;

image_transfer_plan buffer_image_scratch_entry::read(
	array_ref destination,
	const image_transfer_plan &regions
) const
{
	get_host_data(destination);

	const auto &shape = regions.get_shape();
	const auto rank = m_values.get_descriptor().get_layout().get_rank();
	if (shape.get_file_rank() != rank)
	{
		return regions;
	}

	image_transfer_plan held(shape);
	image_transfer_plan missing(shape);
	std::vector<std::size_t> slot_offset(rank);

	const auto index_count = shape.get_extent(rank, 0);
	const auto region_count = regions.get_region_count();
	for (std::size_t region = 0; region < region_count; ++region)
	{
		const auto file_offset = regions.get_file_offset(region);
		const auto array_offset = regions.get_array_offset(region);
		if (!is_present(file_offset.front(), index_count))
		{
			missing.add(file_offset, array_offset);
			continue;
		}

		slot_offset.assign(file_offset.begin(), file_offset.end());
		slot_offset.front() = m_slots.get_lower_bound(file_offset.front());
		held.add(make_span(slot_offset), array_offset);
	}

	copy_regions(const_array_ref(m_values), destination, held);

	return missing;
}

void buffer_image_scratch_entry::store(
	const image_reader &file,
	const image_transfer_plan &regions
)
{
	const auto &shape = regions.get_shape();
	const auto rank = m_values.get_descriptor().get_layout().get_rank();
	if (shape.get_file_rank() != rank)
	{
		return;
	}

	const auto index_count = shape.get_extent(rank, 0);
	const auto region_count = regions.get_region_count();
	for (std::size_t region = 0; region < region_count; ++region)
	{
		const auto first_index = regions.get_file_offset(region).front();
		const auto held_count = m_slots.count_held(first_index, index_count);
		if (held_count == 0)
		{
			continue;
		}

		const auto first_slot = m_slots.get_lower_bound(first_index);
		const auto last_run = m_runs.get_run(first_slot + held_count - 1);
		for (auto run = m_runs.get_run(first_slot); run <= last_run; ++run)
		{
			load(file, run);
		}
	}
}

bool buffer_image_scratch_entry::is_present(
	std::size_t first_index,
	std::size_t index_count
) const noexcept
{
	if (index_count == 0)
	{
		return false;
	}

	if (m_slots.count_held(first_index, index_count) != index_count)
	{
		return false;
	}

	const auto first_slot = m_slots.get_lower_bound(first_index);
	const auto last_run = m_runs.get_run(first_slot + index_count - 1);
	for (auto run = m_runs.get_run(first_slot); run <= last_run; ++run)
	{
		if (!m_runs.is_present(run))
		{
			return false;
		}
	}

	return true;
}

void buffer_image_scratch_entry::load(
	const image_reader &file,
	std::size_t run
)
{
	if (m_runs.is_present(run))
	{
		return;
	}

	const std::lock_guard<std::mutex> lock(m_mutex);
	if (m_runs.is_present(run))
	{
		return;
	}

	file.read(array_ref(m_values), make_run_plan(run));
	m_runs.mark_present(run);
}

image_transfer_plan
buffer_image_scratch_entry::make_run_plan(std::size_t run) const
{
	std::vector<std::size_t> extents;
	m_values.get_descriptor().get_layout().get_extents(extents);

	const auto rank = extents.size();
	image_transfer_plan plan(
		image_transfer_shape(
			std::vector<std::size_t>(extents.begin() + 1, extents.end()),
			rank,
			rank
		)
	);

	const auto first_slot = m_runs.get_first_slot(run);
	const auto slot_count = m_runs.get_slot_count(run);
	plan.reserve(slot_count);

	std::vector<std::size_t> file_offset(rank, 0);
	std::vector<std::size_t> array_offset(rank, 0);
	for (auto slot = first_slot; slot < first_slot + slot_count; ++slot)
	{
		file_offset.front() = m_slots.get_index(slot);
		array_offset.front() = slot;
		plan.add(make_span(file_offset), make_span(array_offset));
	}

	return plan;
}

} // namespace em
} // namespace rexlib
