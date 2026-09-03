// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/interned_path_list.hpp>

#include <rexlib/core/platform/assert.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{

interned_path_list::interned_path_list() noexcept = default;

interned_path_list::interned_path_list(
	const interned_path_list &other
) = default;
interned_path_list::interned_path_list(
	interned_path_list &&other
) noexcept = default;
interned_path_list::~interned_path_list() = default;

interned_path_list&
interned_path_list::operator=(const interned_path_list &other) = default;
interned_path_list&
interned_path_list::operator=(interned_path_list &&other) noexcept = default;

std::size_t interned_path_list::intern(std::string path)
{
	const auto found = find(path);
	if (found != no_path)
	{
		return found;
	}

	m_paths.push_back(std::move(path));
	return m_paths.size() - 1;
}

std::size_t interned_path_list::append(std::size_t path_index)
{
	if (path_index >= m_paths.size())
	{
		throw std::out_of_range(
			"interned_path_list::append: The path index names no interned "
			"path."
		);
	}

	m_entries.push_back(path_index);
	return m_entries.size() - 1;
}

std::size_t interned_path_list::append(std::string path)
{
	return append(intern(std::move(path)));
}

void interned_path_list::clear() noexcept
{
	m_paths.clear();
	m_entries.clear();
}

void interned_path_list::reserve(std::size_t paths, std::size_t entries)
{
	m_paths.reserve(paths);
	m_entries.reserve(entries);
}

std::size_t interned_path_list::find(const std::string &path) const noexcept
{
	const auto ite = std::find(m_paths.begin(), m_paths.end(), path);
	if (ite == m_paths.end())
	{
		return no_path;
	}

	return static_cast<std::size_t>(std::distance(m_paths.begin(), ite));
}

std::size_t interned_path_list::get_size() const noexcept
{
	return m_entries.size();
}

const std::string& interned_path_list::get(std::size_t entry) const noexcept
{
	return get_path(get_path_index(entry));
}

std::size_t
interned_path_list::get_path_index(std::size_t entry) const noexcept
{
	REXLIB_ASSERT(entry < m_entries.size());
	return m_entries[entry];
}

std::size_t interned_path_list::get_path_count() const noexcept
{
	return m_paths.size();
}

const std::string&
interned_path_list::get_path(std::size_t path_index) const noexcept
{
	REXLIB_ASSERT(path_index < m_paths.size());
	return m_paths[path_index];
}

} // namespace em
} // namespace rexlib
