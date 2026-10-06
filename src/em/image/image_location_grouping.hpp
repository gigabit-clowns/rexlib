// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/span.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace rexlib
{
namespace em
{

class image_location;

/**
 * @brief Groups a list of image locations by file.
 *
 * Each file appears once, in the order of its first location. For each
 * file, the grouping stores the stack indices of its locations. It also
 * stores whether any location addresses the file as a whole.
 *
 * Two locations belong to the same file if their paths are equal strings.
 */
class image_location_grouping
{
public:
	/**
	 * @brief Group a list of locations by file.
	 *
	 * @param locations The locations to group.
	 */
	explicit image_location_grouping(span<const image_location> locations);

	image_location_grouping(const image_location_grouping &other) = default;
	image_location_grouping(
		image_location_grouping &&other
	) noexcept = default;
	~image_location_grouping() = default;

	image_location_grouping&
	operator=(const image_location_grouping &other) = default;
	image_location_grouping&
	operator=(image_location_grouping &&other) noexcept = default;

	/**
	 * @brief Get the number of files.
	 *
	 * @return std::size_t The number of files.
	 */
	std::size_t get_file_count() const noexcept;

	/**
	 * @brief Get the path of a file.
	 *
	 * @param file_index Index of the file. Must be below
	 * @ref get_file_count.
	 * @return const std::string& The path. It refers to storage owned by
	 * this grouping.
	 */
	const std::string& get_path(std::size_t file_index) const noexcept;

	/**
	 * @brief Check whether any location addresses a file as a whole.
	 *
	 * @param file_index Index of the file. Must be below
	 * @ref get_file_count.
	 * @return true At least one location of the file has no stack index.
	 * @return false All locations of the file have a stack index.
	 */
	bool is_whole(std::size_t file_index) const noexcept;

	/**
	 * @brief Get the stack indices of the locations of a file.
	 *
	 * @param file_index Index of the file. Must be below
	 * @ref get_file_count.
	 * @return span<const std::size_t> The indices, in ascending order and
	 * without repeats. Empty if no location of the file has a stack index.
	 * It refers to storage owned by this grouping.
	 */
	span<const std::size_t>
	get_indices(std::size_t file_index) const noexcept;

private:
	std::vector<std::string> m_paths;
	std::vector<bool> m_whole;
	std::vector<std::size_t> m_indices;
	std::vector<std::size_t> m_first_positions;
};

} // namespace em
} // namespace rexlib
