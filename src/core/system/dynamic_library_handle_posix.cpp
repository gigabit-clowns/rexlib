// SPDX-License-Identifier: GPL-3.0-only

#include "dynamic_library_handle.hpp"

#include <rexlib/core/exceptions/dynamic_library_error.hpp>

#include <dlfcn.h>

namespace rexlib
{

void* dynamic_library_open(const char* filename)
{
	auto *const handle = ::dlopen(filename, RTLD_NOW | RTLD_LOCAL);
	if (handle == nullptr)
	{
		const char *const reason = ::dlerror();
		throw dynamic_library_error(
			reason != nullptr
				? std::string(reason)
				: std::string(filename) + ": could not be loaded"
		);
	}
	return handle;
}

void dynamic_library_close(void* handle) noexcept
{
	::dlclose(handle);
}

void* dynamic_library_get_symbol(void* handle, const char* name) noexcept
{
	return ::dlsym(handle, name);
}

std::string dynamic_library_symbol_filename_lookup(const void* symbol)
{
	Dl_info info;
	if (::dladdr(symbol, &info) == 0 || info.dli_fname == nullptr)
	{
		return std::string();
	}
	return std::string(info.dli_fname);
}

} // namespace rexlib
