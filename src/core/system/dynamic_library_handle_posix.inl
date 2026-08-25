// SPDX-License-Identifier: GPL-3.0-only

#include "dynamic_library_handle.hpp"

#include <rexlib/core/platform/constexpr.hpp>

#include <dlfcn.h>

#include <system_error>

namespace rexlib
{

inline void* dynamic_library_open(const char* filename)
{
	REXLIB_CONST_CONSTEXPR int flags = RTLD_LAZY;
	auto *const result = ::dlopen(filename, flags);
	if (result == nullptr)
	{
		throw std::system_error(std::error_code(), dlerror());
	}
	return result;
}

inline void dynamic_library_close(void* handle) noexcept
{
	::dlclose(handle);
}

inline void* dynamic_library_get_symbol(void* handle, const char* name) noexcept
{
	return ::dlsym(handle, name);
}

inline std::string dynamic_library_symbol_filename_lookup(const void* symbol)
{
	std::string result;

	Dl_info info;
	if (dladdr(symbol, &info))
	{
		result = std::string(info.dli_fname);
	}

	return result;
}

} // namespace rexlib
