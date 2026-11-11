// SPDX-License-Identifier: GPL-3.0-only

#include "utf8_path.hpp"

#include <rexlib/core/platform/operating_system.h>

#if REXLIB_WINDOWS
	#include "utf16_windows.hpp"
#endif

namespace rexlib
{

boost::filesystem::path utf8_to_path(const std::string& utf8)
{
	#if REXLIB_WINDOWS
		return boost::filesystem::path(utf8_to_utf16(utf8));
	#else
		return boost::filesystem::path(utf8);
	#endif
}

std::string path_to_utf8(const boost::filesystem::path& path)
{
	#if REXLIB_WINDOWS
		return utf16_to_utf8(path.wstring());
	#else
		return path.string();
	#endif
}

} // namespace rexlib
