// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace rexlib
{
namespace em
{

/**
 * @brief A list of paths in which equal paths cost one string.
 *
 * A transaction names one file per region, and the same file over and over:
 * a batch of two hundred and fifty six elements drawn from eight stacks
 * names eight distinct paths. Holding a string per entry would copy each of
 * them as many times as it is named, so the distinct paths are held once and
 * the entries are indices into them.
 *
 * The list therefore has two sizes. @ref get_path_count is how many distinct
 * paths were interned and @ref get_size how many entries refer to them, and
 * the second is the larger one. Both are addressed separately:
 * @ref get_path takes a path index, @ref get takes an entry.
 *
 * @ref clear keeps both capacities and the interned paths are dropped with
 * them, so one instance reused from one call to the next allocates nothing
 * after the first beyond the strings it has not seen before.
 */
class interned_path_list
{
public:
	/**
	 * @brief Index reported for a path that was never interned.
	 */
	static REXLIB_INLINE_CONST_CONSTEXPR std::size_t no_path =
		std::numeric_limits<std::size_t>::max();

	/**
	 * @brief Construct a list holding no path and no entry.
	 */
	REXLIB_API
	interned_path_list() noexcept;

	REXLIB_API
	interned_path_list(const interned_path_list &other);
	REXLIB_API
	interned_path_list(interned_path_list &&other) noexcept;
	REXLIB_API
	~interned_path_list();

	REXLIB_API
	interned_path_list& operator=(const interned_path_list &other);
	REXLIB_API
	interned_path_list& operator=(interned_path_list &&other) noexcept;

	/**
	 * @brief Intern a path without appending an entry.
	 *
	 * A path equal to one already interned yields the index it was given
	 * the first time rather than a second one, which is what lets a caller
	 * name a file once and then refer to it by index.
	 *
	 * The interned paths are searched one by one, so this costs what the
	 * number of distinct paths costs rather than what the number of entries
	 * does. Those are the files one transaction touches, which the stack
	 * model keeps small however many elements the transaction carries.
	 *
	 * @param path The path to intern.
	 * @return std::size_t Index of the path, below @ref get_path_count.
	 */
	REXLIB_API
	std::size_t intern(std::string path);

	/**
	 * @brief Append one entry referring to an interned path.
	 *
	 * @param path_index Index of the path, as @ref intern returned it. Must
	 * be below @ref get_path_count.
	 * @return std::size_t Position of the appended entry.
	 * @throws std::out_of_range If @p path_index names no interned path.
	 */
	REXLIB_API
	std::size_t append(std::size_t path_index);

	/**
	 * @brief Intern a path and append one entry referring to it.
	 *
	 * @param path The path to intern and refer to.
	 * @return std::size_t Position of the appended entry.
	 */
	REXLIB_API
	std::size_t append(std::string path);

	/**
	 * @brief Drop every path and every entry, keeping both capacities.
	 */
	REXLIB_API
	void clear() noexcept;

	/**
	 * @brief Make room without allocating later.
	 *
	 * @param paths Number of distinct paths to make room for.
	 * @param entries Number of entries to make room for.
	 */
	REXLIB_API
	void reserve(std::size_t paths, std::size_t entries);

	/**
	 * @brief Get the index a path was interned under.
	 *
	 * @param path The path to look up.
	 * @return std::size_t Its index, or @ref no_path when it was never
	 * interned.
	 */
	REXLIB_API
	std::size_t find(const std::string &path) const noexcept;

	/**
	 * @brief Get how many entries are held.
	 *
	 * @return std::size_t The number of entries.
	 */
	REXLIB_API
	std::size_t get_size() const noexcept;

	/**
	 * @brief Get the path one entry refers to.
	 *
	 * @param entry Position of the entry. Must be below @ref get_size.
	 * @return const std::string& The path. It refers to storage owned by
	 * this list, which interning into, assigning to or destroying it
	 * invalidates.
	 */
	REXLIB_API
	const std::string& get(std::size_t entry) const noexcept;

	/**
	 * @brief Get which interned path one entry refers to.
	 *
	 * @param entry Position of the entry. Must be below @ref get_size.
	 * @return std::size_t The path index, below @ref get_path_count.
	 */
	REXLIB_API
	std::size_t get_path_index(std::size_t entry) const noexcept;

	/**
	 * @brief Get how many distinct paths are held.
	 *
	 * Never above @ref get_size, and below it as soon as one path is named
	 * twice.
	 *
	 * @return std::size_t The number of distinct paths.
	 */
	REXLIB_API
	std::size_t get_path_count() const noexcept;

	/**
	 * @brief Get one of the interned paths.
	 *
	 * @param path_index Index of the path. Must be below
	 * @ref get_path_count.
	 * @return const std::string& The path. It refers to storage owned by
	 * this list, which interning into, assigning to or destroying it
	 * invalidates.
	 */
	REXLIB_API
	const std::string& get_path(std::size_t path_index) const noexcept;

private:
	std::vector<std::string> m_paths;
	std::vector<std::size_t> m_entries;
};

} // namespace em
} // namespace rexlib
