// SPDX-License-Identifier: GPL-3.0-only

#include "environment.hpp"

#include <rexlib/core/platform/operating_system.h>

#if REXLIB_WINDOWS
	#include "utf16_windows.hpp"
#endif

#include <cstdlib>

namespace rexlib
{

std::string get_environment_variable(const std::string& name)
{
	#if REXLIB_WINDOWS
		const wchar_t *const value = ::_wgetenv(utf8_to_utf16(name).c_str());
		return value != nullptr ? utf16_to_utf8(value) : std::string();
	#else
		const char *const value = std::getenv(name.c_str());
		return value != nullptr ? std::string(value) : std::string();
	#endif
}

} // namespace rexlib
