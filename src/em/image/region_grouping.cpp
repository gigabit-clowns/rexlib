// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/region_grouping.hpp>

#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>

#include <numeric>

namespace rexlib
{
namespace em
{

region_grouping::region_grouping() noexcept = default;

region_grouping::region_grouping(const region_grouping &other) = default;
region_grouping::region_grouping(
	region_grouping &&other
) noexcept = default;
region_grouping::~region_grouping() = default;

region_grouping&
region_grouping::operator=(const region_grouping &other) = default;
region_grouping&
region_grouping::operator=(region_grouping &&other) noexcept = default;

void region_grouping::build(const image_transaction_plan &plan)
{
	const auto files = plan.get_file_count();
	const auto regions = plan.get_size();

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

void region_grouping::clear() noexcept
{
	m_regions.clear();
	m_first_position.assign(1, 0);
}

void region_grouping::reserve(std::size_t files, std::size_t regions)
{
	m_regions.reserve(regions);
	m_first_position.reserve(files + 1);
	m_cursors.reserve(files);
}

std::size_t region_grouping::get_size() const noexcept
{
	return m_regions.size();
}

std::size_t region_grouping::get_file_count() const noexcept
{
	return m_first_position.empty() ? 0 : m_first_position.size() - 1;
}

std::size_t
region_grouping::get_first_position(std::size_t file_index) const noexcept
{
	REXLIB_ASSERT(file_index < get_file_count());
	return m_first_position[file_index];
}

std::size_t
region_grouping::get_region_count(std::size_t file_index) const noexcept
{
	REXLIB_ASSERT(file_index < get_file_count());
	return m_first_position[file_index + 1] - m_first_position[file_index];
}

std::size_t region_grouping::get_region(std::size_t position) const noexcept
{
	REXLIB_ASSERT(position < m_regions.size());
	return m_regions[position];
}

} // namespace em
} // namespace rexlib
