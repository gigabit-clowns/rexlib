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
 * Each file has one group, and the groups are in ascending order of path.
 * The grouping therefore does not depend on the order of the locations.
 *
 * A file is either addressed as a whole, or has the stack indices of its
 * locations. A file that any location addresses as a whole has no indices,
 * because the whole file includes them.
 *
 * Two locations belong to the same file if their paths are equal strings.
 */
class image_location_grouping
{
public:
	class group;

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
	 * @brief Get the number of groups.
	 *
	 * @return std::size_t The number of groups, which is the number of
	 * files.
	 */
	REXLIB_API
	std::size_t get_group_count() const noexcept;

	/**
	 * @brief Get the group of a file.
	 *
	 * @param index Index of the group.
	 * @return group The group. It refers to storage owned by this grouping.
	 * @throws std::out_of_range If @p index is not below
	 * @ref get_group_count.
	 */
	REXLIB_API
	group get_group(std::size_t index) const;

private:
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::string> m_paths;
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::size_t> m_indices;
	REXLIB_STD_MEMBER_INTERFACE
	std::vector<std::size_t> m_first_positions;
};

/**
 * @brief The locations of one file in an @ref image_location_grouping.
 *
 * A group refers to storage owned by its grouping, so it can be used only
 * while the grouping is alive and has not been assigned to.
 */
class image_location_grouping::group
{
public:
	/**
	 * @brief Get the path of the file.
	 *
	 * @return const std::string& The path.
	 */
	REXLIB_API
	const std::string& get_path() const noexcept;

	/**
	 * @brief Check whether any location addresses the file as a whole.
	 *
	 * @return true At least one location of the file has no stack index.
	 * @return false All locations of the file have a stack index.
	 */
	REXLIB_API
	bool is_whole() const noexcept;

	/**
	 * @brief Get the stack indices of the locations of the file.
	 *
	 * @return span<const std::size_t> The indices, in ascending order and
	 * without repeats. Empty if the file is addressed as a whole.
	 */
	REXLIB_API
	span<const std::size_t> get_indices() const noexcept;

private:
	friend class image_location_grouping;

	group(const std::string &path, span<const std::size_t> indices) noexcept;

	const std::string *m_path;
	span<const std::size_t> m_indices;
};

} // namespace em
} // namespace rexlib
