// SPDX-License-Identifier: GPL-3.0-only

#include "image_location_grouping.hpp"

#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_location.hpp>

#include <algorithm>
#include <unordered_map>

namespace rexlib
{
namespace em
{

image_location_grouping::image_location_grouping(
	span<const image_location> locations
)
{
	std::unordered_map<std::string, std::size_t> files;
	std::vector<std::vector<std::size_t>> named;

	for (const auto &location : locations)
	{
		const auto inserted =
			files.emplace(location.get_path(), m_paths.size());
		if (inserted.second)
		{
			m_paths.push_back(location.get_path());
			m_whole.push_back(false);
			named.emplace_back();
		}

		const auto file_index = inserted.first->second;
		if (location.has_index_in_stack())
		{
			named[file_index].push_back(location.get_index_in_stack());
		}
		else
		{
			m_whole[file_index] = true;
		}
	}

	m_first_positions.reserve(named.size() + 1);
	m_first_positions.push_back(0);
	for (auto &indices : named)
	{
		std::sort(indices.begin(), indices.end());
		m_indices.insert(
			m_indices.end(),
			indices.begin(),
			std::unique(indices.begin(), indices.end())
		);
		m_first_positions.push_back(m_indices.size());
	}
}

std::size_t image_location_grouping::get_file_count() const noexcept
{
	return m_paths.size();
}

const std::string&
image_location_grouping::get_path(std::size_t file_index) const noexcept
{
	REXLIB_ASSERT(file_index < get_file_count());
	return m_paths[file_index];
}

bool image_location_grouping::is_whole(std::size_t file_index) const noexcept
{
	REXLIB_ASSERT(file_index < get_file_count());
	return m_whole[file_index];
}

span<const std::size_t>
image_location_grouping::get_indices(std::size_t file_index) const noexcept
{
	REXLIB_ASSERT(file_index < get_file_count());

	const auto first = m_first_positions[file_index];
	const auto last = m_first_positions[file_index + 1];

	return make_span(m_indices.data() + first, last - first);
}

} // namespace em
} // namespace rexlib
