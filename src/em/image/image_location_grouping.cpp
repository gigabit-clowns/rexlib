// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_location_grouping.hpp>

#include <rexlib/em/image/image_location.hpp>

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace rexlib
{
namespace em
{

image_location_grouping::image_location_grouping(
	span<const image_location> locations
)
{
	std::unordered_map<std::string, std::size_t> files;
	std::vector<std::string> paths;
	std::vector<std::vector<std::size_t>> named;
	std::vector<bool> whole;

	for (const auto &location : locations)
	{
		const auto inserted = files.emplace(location.get_path(), paths.size());
		if (inserted.second)
		{
			paths.push_back(location.get_path());
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

	std::vector<std::size_t> order(paths.size());
	std::iota(order.begin(), order.end(), std::size_t(0));
	std::sort(
		order.begin(),
		order.end(),
		[&paths] (std::size_t lhs, std::size_t rhs)
		{
			return paths[lhs] < paths[rhs];
		}
	);

	m_paths.reserve(paths.size());
	m_first_positions.reserve(paths.size() + 1);
	m_first_positions.push_back(0);
	for (const auto file_index : order)
	{
		m_paths.push_back(std::move(paths[file_index]));

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

std::size_t image_location_grouping::get_group_count() const noexcept
{
	return m_paths.size();
}

image_location_grouping::group
image_location_grouping::get_group(std::size_t index) const
{
	if (index >= get_group_count())
	{
		throw std::out_of_range(
			"image_location_grouping: The index is not below the number of "
			"groups."
		);
	}

	const auto first = m_first_positions[index];
	const auto last = m_first_positions[index + 1];

	return group(
		m_paths[index],
		make_span(m_indices.data() + first, last - first)
	);
}

image_location_grouping::group::group(
	const std::string &path,
	span<const std::size_t> indices
) noexcept
	: m_path(&path)
	, m_indices(indices)
{
}

const std::string& image_location_grouping::group::get_path() const noexcept
{
	REXLIB_ASSERT(m_path);
	return *m_path;
}

bool image_location_grouping::group::is_whole() const noexcept
{
	return m_indices.empty();
}

span<const std::size_t>
image_location_grouping::group::get_indices() const noexcept
{
	return m_indices;
}

} // namespace em
} // namespace rexlib
