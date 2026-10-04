// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/tests/assets.hpp>

#include <cstdio>
#include <string>

namespace rexlib
{

/**
 * @brief A path under the build tree that is removed when a case leaves it,
 * whether it succeeded or not.
 *
 * The scratch directory is shared and every case runs as a process of its
 * own, so two cases naming the same file race: one truncates what the other
 * has mapped. No two names may repeat across the suite.
 */
class scoped_path
{
public:
	explicit scoped_path(const std::string &name)
		: m_path(get_scratch_path(name))
	{
		std::remove(m_path.c_str());
	}

	scoped_path(const scoped_path &other) = delete;
	scoped_path(scoped_path &&other) = delete;

	~scoped_path()
	{
		std::remove(m_path.c_str());
	}

	scoped_path& operator=(const scoped_path &other) = delete;
	scoped_path& operator=(scoped_path &&other) = delete;

	const std::string& get() const noexcept
	{
		return m_path;
	}

private:
	std::string m_path;
};

} // namespace rexlib
