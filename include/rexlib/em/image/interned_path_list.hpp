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
 * @brief A list of paths in which equal paths are held once.
 *
 * Each entry of the list refers to a path, and entries naming the same path
 * share one copy of it: the distinct paths are held once and the entries are
 * indices into them. A list naming a few paths many times over therefore
 * costs a string per distinct path rather than per entry.
 *
 * The list has two sizes. @ref get_path_count is how many distinct paths
 * were interned and @ref get_entry_count how many entries there are. Each is
 * addressed on its own: @ref get_path takes a path index, @ref get an entry
 * index.
 *
 * @ref clear keeps both capacities, so a list refilled after clearing
 * allocates only the strings of the paths it interns.
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
	 * the first time rather than a second one, so a path can be interned
	 * once and referred to by index afterwards.
	 *
	 * The interned paths are searched one by one, so this takes time in
	 * proportion to the number of distinct paths, not of entries.
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
	std::size_t get_entry_count() const noexcept;

	/**
	 * @brief Get the path one entry refers to.
	 *
	 * @param entry_index Position of the entry. Must be below
	 * @ref get_entry_count.
	 * @return const std::string& The path. It refers to storage owned by
	 * this list, which interning into, assigning to or destroying it
	 * invalidates.
	 */
	REXLIB_API
	const std::string& get(std::size_t entry_index) const noexcept;

	/**
	 * @brief Get which interned path one entry refers to.
	 *
	 * @param entry_index Position of the entry. Must be below
	 * @ref get_entry_count.
	 * @return std::size_t The path index, below @ref get_path_count.
	 */
	REXLIB_API
	std::size_t get_path_index(std::size_t entry_index) const noexcept;

	/**
	 * @brief Get how many distinct paths are held.
	 *
	 * A path counts once however many entries refer to it, and also when
	 * none does, having been interned without an entry.
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
