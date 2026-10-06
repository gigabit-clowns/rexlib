// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>
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
 * Each file appears once, in the order of its first location. A file is
 * either addressed as a whole, or has the stack indices of its locations.
 * A file that any location addresses as a whole has no indices, because
 * the whole file includes them.
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
	REXLIB_API
	explicit image_location_grouping(span<const image_location> locations);

	REXLIB_API
	image_location_grouping(const image_location_grouping &other);
	REXLIB_API
	image_location_grouping(image_location_grouping &&other) noexcept;
	REXLIB_API
	~image_location_grouping();

	REXLIB_API
	image_location_grouping&
	operator=(const image_location_grouping &other);
	REXLIB_API
	image_location_grouping&
	operator=(image_location_grouping &&other) noexcept;

	/**
	 * @brief Get the number of files.
	 *
	 * @return std::size_t The number of files.
	 */
	REXLIB_API
	std::size_t get_file_count() const noexcept;

	/**
	 * @brief Get the path of a file.
	 *
	 * @param file_index Index of the file.
	 * @return const std::string& The path. It refers to storage owned by
	 * this grouping.
	 * @throws std::out_of_range If @p file_index is not below
	 * @ref get_file_count.
	 */
	REXLIB_API
	const std::string& get_path(std::size_t file_index) const;

	/**
	 * @brief Check whether any location addresses a file as a whole.
	 *
	 * @param file_index Index of the file.
	 * @return true At least one location of the file has no stack index.
	 * @return false All locations of the file have a stack index.
	 * @throws std::out_of_range If @p file_index is not below
	 * @ref get_file_count.
	 */
	REXLIB_API
	bool is_whole(std::size_t file_index) const;

	/**
	 * @brief Get the stack indices of the locations of a file.
	 *
	 * @param file_index Index of the file.
	 * @return span<const std::size_t> The indices, in ascending order and
	 * without repeats. Empty if the file is addressed as a whole. It refers
	 * to storage owned by this grouping.
	 * @throws std::out_of_range If @p file_index is not below
	 * @ref get_file_count.
	 */
	REXLIB_API
	span<const std::size_t> get_indices(std::size_t file_index) const;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::string> m_paths;
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::size_t> m_indices;
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::size_t> m_first_positions;
};

} // namespace em
} // namespace rexlib
