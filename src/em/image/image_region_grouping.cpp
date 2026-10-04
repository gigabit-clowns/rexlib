// SPDX-License-Identifier: GPL-3.0-only

#include <em/image/image_region_grouping.hpp>

#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <numeric>

namespace rexlib
{
namespace em
{

image_region_grouping::image_region_grouping() noexcept = default;

image_region_grouping::image_region_grouping(
	const image_region_grouping &other
) = default;
image_region_grouping::image_region_grouping(
	image_region_grouping &&other
) noexcept = default;
image_region_grouping::~image_region_grouping() = default;

image_region_grouping&
image_region_grouping::operator=(const image_region_grouping &other) = default;
image_region_grouping&
image_region_grouping::operator=(
	image_region_grouping &&other
) noexcept = default;

void image_region_grouping::build(const image_transaction_plan &plan)
{
	const auto files = plan.get_file_count();
	const auto regions = plan.get_region_count();

	m_first_position.assign(files + 1, 0);
	for (std::size_t region = 0; region < regions; ++region)
	{
		++m_first_position[plan.get_region_file(region) + 1];
	}
	std::partial_sum(
		m_first_position.begin(),
		m_first_position.end(),
		m_first_position.begin()
	);

	m_cursors.assign(m_first_position.begin(), m_first_position.end() - 1);
	m_regions.resize(regions);
	for (std::size_t region = 0; region < regions; ++region)
	{
		m_regions[m_cursors[plan.get_region_file(region)]++] = region;
	}
}

void image_region_grouping::clear() noexcept
{
	m_regions.clear();
	m_first_position.assign(1, 0);
}

void image_region_grouping::reserve(std::size_t files, std::size_t regions)
{
	m_regions.reserve(regions);
	m_first_position.reserve(files + 1);
	m_cursors.reserve(files);
}

std::size_t image_region_grouping::get_region_count() const noexcept
{
	return m_regions.size();
}

std::size_t image_region_grouping::get_file_count() const noexcept
{
	return m_first_position.empty() ? 0 : m_first_position.size() - 1;
}

std::size_t image_region_grouping::get_addressed_file_count() const noexcept
{
	std::size_t addressed_file_count = 0;
	const auto file_count = get_file_count();
	for (std::size_t file_index = 0; file_index < file_count; ++file_index)
	{
		if (get_file_region_count(file_index) > 0)
		{
			++addressed_file_count;
		}
	}

	return addressed_file_count;
}

std::size_t
image_region_grouping::get_first_position(std::size_t file_index) const noexcept
{
	REXLIB_ASSERT(file_index < get_file_count());
	return m_first_position[file_index];
}

std::size_t
image_region_grouping::get_file_region_count(
	std::size_t file_index
) const noexcept
{
	REXLIB_ASSERT(file_index < get_file_count());
	return m_first_position[file_index + 1] - m_first_position[file_index];
}

std::size_t
image_region_grouping::get_region(std::size_t position) const noexcept
{
	REXLIB_ASSERT(position < m_regions.size());
	return m_regions[position];
}

image_transfer_plan make_file_transfer_plan(
	const image_region_grouping &grouping,
	const image_transaction_plan &plan,
	std::size_t file_index
)
{
	const auto first = grouping.get_first_position(file_index);
	const auto count = grouping.get_file_region_count(file_index);

	image_transfer_plan transfer(plan.get_shape());
	transfer.reserve(count);
	for (auto i = first; i < first + count; ++i)
	{
		const auto region = grouping.get_region(i);
		transfer.add(
			plan.get_file_offset(region),
			plan.get_array_offset(region)
		);
	}

	return transfer;
}

} // namespace em
} // namespace rexlib
