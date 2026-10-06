// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_location_grouping.hpp>

#include <rexlib/em/image/image_location.hpp>

#include <algorithm>
#include <stdexcept>
#include <unordered_map>

namespace rexlib
{
namespace em
{

namespace
{

void check_file_index(std::size_t file_index, std::size_t file_count)
{
	if (file_index >= file_count)
	{
		throw std::out_of_range(
			"image_location_grouping: The file index is not below the number "
			"of files."
		);
	}
}

} // anonymous namespace

image_location_grouping::image_location_grouping(
	span<const image_location> locations
)
{
	std::unordered_map<std::string, std::size_t> files;
	std::vector<std::vector<std::size_t>> named;
	std::vector<bool> whole;

	for (const auto &location : locations)
	{
		const auto inserted =
			files.emplace(location.get_path(), m_paths.size());
		if (inserted.second)
		{
			m_paths.push_back(location.get_path());
			named.emplace_back();
			whole.push_back(false);
		}

		const auto file_index = inserted.first->second;
		if (location.has_index_in_stack())
		{
			named[file_index].push_back(location.get_index_in_stack());
		}
		else
		{
			whole[file_index] = true;
		}
	}

	m_first_positions.reserve(named.size() + 1);
	m_first_positions.push_back(0);
	for (std::size_t file_index = 0; file_index < named.size(); ++file_index)
	{
		if (!whole[file_index])
		{
			auto &indices = named[file_index];
			std::sort(indices.begin(), indices.end());
			m_indices.insert(
				m_indices.end(),
				indices.begin(),
				std::unique(indices.begin(), indices.end())
			);
		}

		m_first_positions.push_back(m_indices.size());
	}
}

image_location_grouping::image_location_grouping(
	const image_location_grouping &other
) = default;
image_location_grouping::image_location_grouping(
	image_location_grouping &&other
) noexcept = default;
image_location_grouping::~image_location_grouping() = default;

image_location_grouping& image_location_grouping::operator=(
	const image_location_grouping &other
) = default;
image_location_grouping& image_location_grouping::operator=(
	image_location_grouping &&other
) noexcept = default;

std::size_t image_location_grouping::get_file_count() const noexcept
{
	return m_paths.size();
}

const std::string&
image_location_grouping::get_path(std::size_t file_index) const
{
	check_file_index(file_index, get_file_count());
	return m_paths[file_index];
}

bool image_location_grouping::is_whole(std::size_t file_index) const
{
	return get_indices(file_index).empty();
}

span<const std::size_t>
image_location_grouping::get_indices(std::size_t file_index) const
{
	check_file_index(file_index, get_file_count());

	const auto first = m_first_positions[file_index];
	const auto last = m_first_positions[file_index + 1];

	return make_span(m_indices.data() + first, last - first);
}

} // namespace em
} // namespace rexlib
