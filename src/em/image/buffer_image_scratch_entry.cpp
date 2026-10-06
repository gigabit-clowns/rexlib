// SPDX-License-Identifier: GPL-3.0-only

#include "buffer_image_scratch_entry.hpp"

#include <em/image/formats/strided_transfer/image_host_access.hpp>
#include <em/image/formats/strided_transfer/image_region_copy.hpp>

#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{

namespace
{

std::size_t
compute_run_count(std::size_t slot_count, std::size_t run_length)
{
	if (run_length == 0)
	{
		throw std::invalid_argument(
			"buffer_image_scratch_entry: A run must span at least one slot."
		);
	}

	if (slot_count == 0)
	{
		return 0;
	}

	return (slot_count - 1) / run_length + 1;
}

// A range that reaches past the largest index ends at it, which is an index
// that no file has.
std::size_t
compute_end_index(std::size_t first_index, std::size_t index_count) noexcept
{
	const auto max_index = std::numeric_limits<std::size_t>::max();
	return first_index + std::min(index_count, max_index - first_index);
}

} // anonymous namespace

buffer_image_scratch_entry::buffer_image_scratch_entry(
	std::vector<std::size_t> indices,
	array values,
	std::size_t run_length
)
	: m_indices(std::move(indices))
	, m_run_length(run_length)
	, m_loaded(compute_run_count(m_indices.size(), run_length))
	, m_values(std::move(values))
{
	const auto unordered = std::adjacent_find(
		m_indices.begin(),
		m_indices.end(),
		std::greater_equal<std::size_t>()
	);
	if (unordered != m_indices.end())
	{
		throw std::invalid_argument(
			"buffer_image_scratch_entry: The indices are not strictly "
			"ascending."
		);
	}

	get_host_data(const_array_ref(m_values));

	std::vector<std::size_t> extents;
	m_values.get_descriptor().get_layout().get_extents(extents);
	if (extents.empty() || extents.front() != m_indices.size())
	{
		throw std::invalid_argument(
			"buffer_image_scratch_entry: The first extent of the values is "
			"not the number of indices."
		);
	}

	for (auto &loaded : m_loaded)
	{
		loaded.store(false, std::memory_order_relaxed);
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

	image_transfer_plan loaded(shape);
	image_transfer_plan missing(shape);
	std::vector<std::size_t> slot_offset(rank);

	const auto index_count = shape.get_extent(rank, 0);
	const auto region_count = regions.get_region_count();
	for (std::size_t region = 0; region < region_count; ++region)
	{
		const auto file_offset = regions.get_file_offset(region);
		const auto array_offset = regions.get_array_offset(region);

		const auto first_index = file_offset.front();
		const auto first_slot = find_slot(first_index);
		const auto end_slot =
			find_slot(compute_end_index(first_index, index_count));
		const auto is_held =
			index_count > 0 && end_slot - first_slot == index_count;
		if (!is_held || !are_loaded(first_slot, end_slot))
		{
			missing.add(file_offset, array_offset);
			continue;
		}

		slot_offset.assign(file_offset.begin(), file_offset.end());
		slot_offset.front() = first_slot;
		loaded.add(make_span(slot_offset), array_offset);
	}

	copy_regions(const_array_ref(m_values), destination, loaded);

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
		const auto first_slot = find_slot(first_index);
		const auto end_slot =
			find_slot(compute_end_index(first_index, index_count));
		if (first_slot == end_slot)
		{
			continue;
		}

		const auto last_run = (end_slot - 1) / m_run_length;
		for (auto run = first_slot / m_run_length; run <= last_run; ++run)
		{
			load(file, run);
		}
	}
}

std::size_t
buffer_image_scratch_entry::find_slot(std::size_t index) const noexcept
{
	const auto ite =
		std::lower_bound(m_indices.begin(), m_indices.end(), index);
	return static_cast<std::size_t>(std::distance(m_indices.begin(), ite));
}

bool buffer_image_scratch_entry::are_loaded(
	std::size_t first_slot,
	std::size_t end_slot
) const noexcept
{
	REXLIB_ASSERT(first_slot < end_slot);
	REXLIB_ASSERT(end_slot <= m_indices.size());

	const auto last_run = (end_slot - 1) / m_run_length;
	for (auto run = first_slot / m_run_length; run <= last_run; ++run)
	{
		if (!m_loaded[run].load(std::memory_order_acquire))
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
	REXLIB_ASSERT(run < m_loaded.size());

	if (m_loaded[run].load(std::memory_order_acquire))
	{
		return;
	}

	const std::lock_guard<std::mutex> lock(m_mutex);
	if (m_loaded[run].load(std::memory_order_acquire))
	{
		return;
	}

	file.read(array_ref(m_values), make_load_plan(run));
	m_loaded[run].store(true, std::memory_order_release);
}

image_transfer_plan
buffer_image_scratch_entry::make_load_plan(std::size_t run) const
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

	const auto first_slot = run * m_run_length;
	const auto slot_count =
		std::min(m_run_length, m_indices.size() - first_slot);
	plan.reserve(slot_count);

	std::vector<std::size_t> file_offset(rank, 0);
	std::vector<std::size_t> array_offset(rank, 0);
	for (std::size_t slot = first_slot; slot < first_slot + slot_count; ++slot)
	{
		file_offset.front() = m_indices[slot];
		array_offset.front() = slot;
		plan.add(make_span(file_offset), make_span(array_offset));
	}

	return plan;
}

} // namespace em
} // namespace rexlib
