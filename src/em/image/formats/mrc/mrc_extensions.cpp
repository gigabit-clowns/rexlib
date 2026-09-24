// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_extensions.hpp"

#include <algorithm>
#include <array>

namespace rexlib
{
namespace em
{
namespace mrc
{

namespace
{

template <std::size_t N>
bool contains(
	const std::array<const char*, N> &extensions,
	const std::string &extension
) noexcept
{
	return std::any_of(
		extensions.cbegin(),
		extensions.cend(),
		[&extension] (const char *candidate)
		{
			return extension == candidate;
		}
	);
}

} // anonymous namespace

bool is_readable_extension(const std::string &extension) noexcept
{
	static const std::array<const char*, 6> extensions = {{
		".mrc", ".mrcs", ".map",
		".st", ".rec", ".ali" // IMOD <4.11
	}};

	return contains(extensions, extension);
}

bool is_writable_extension(const std::string &extension) noexcept
{
	static const std::array<const char*, 3> extensions = {{
		".mrc", ".mrcs", ".map"
	}};

	return contains(extensions, extension);
}

mrc_single_section get_single_section(const std::string &extension) noexcept
{
	return extension == ".mrcs"
		? mrc_single_section::image_stack
		: mrc_single_section::image;
}

} // namespace mrc
} // namespace em
} // namespace rexlib
