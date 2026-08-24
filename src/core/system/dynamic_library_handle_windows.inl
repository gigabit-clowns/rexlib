// SPDX-License-Identifier: GPL-3.0-only

#include "dynamic_library_handle.hpp"

#include <windows.h>

#include <system_error>

namespace rex
{

inline void* dynamic_library_open(const char* filename)
{
	DWORD prev_error_mode;
	::SetThreadErrorMode(
		SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX,
		&prev_error_mode
	);
	const auto result = static_cast<void*>(::LoadLibrary(filename));
	::SetThreadErrorMode(prev_error_mode, nullptr);
	if (result == NULL)
	{
		throw std::system_error(
			std::error_code(::GetLastError(), std::system_category())
		);
	}
	return result;
}

inline void dynamic_library_close(void* handle) noexcept
{
	::FreeLibrary(static_cast<HMODULE>(handle));
}

inline void* dynamic_library_get_symbol(void* handle, const char* name) noexcept
{
	return reinterpret_cast<void*>(
		::GetProcAddress(static_cast<HMODULE>(handle), name)
	);
}

inline std::string dynamic_library_symbol_filename_lookup(const void* symbol)
{
	std::string result;

	HMODULE module;
	REXLIB_CONST_CONSTEXPR DWORD flags = 
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT ;

	if (::GetModuleHandleEx(flags, reinterpret_cast<LPCTSTR>(symbol), &module)) 
	{
		char path[MAX_PATH];
		int count;
		if((count = ::GetModuleFileNameA(module, path, MAX_PATH)))
		{
			result = std::string(path, count);
		}
	}

	return result;
}

} // namespace rex
