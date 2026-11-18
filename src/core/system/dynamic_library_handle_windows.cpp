// SPDX-License-Identifier: GPL-3.0-only

#include "dynamic_library_handle.hpp"

#include "utf16_windows.hpp"

#include <rexlib/core/exceptions/dynamic_library_error.hpp>

#include <windows.h>

#include <array>
#include <string>

namespace rexlib
{

namespace
{

std::string get_system_error_message(DWORD error)
{
	std::array<wchar_t, 1024> buffer = {};
	const DWORD length = ::FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		error,
		0,
		buffer.data(),
		static_cast<DWORD>(buffer.size()),
		nullptr
	);
	if (length == 0)
	{
		return "error " + std::to_string(error);
	}

	std::wstring message(buffer.data(), length);
	const auto end = message.find_last_not_of(L" \r\n");
	message.erase(end == std::wstring::npos ? 0 : end + 1);
	return utf16_to_utf8(message);
}

std::wstring get_full_path(const std::string& filename)
{
	const std::wstring path = utf8_to_utf16(filename);
	const DWORD size = ::GetFullPathNameW(path.c_str(), 0, nullptr, nullptr);
	std::wstring result(size, L'\0');
	const DWORD length = size == 0 ? 0 : ::GetFullPathNameW(
		path.c_str(), size, &result[0], nullptr
	);
	if (length == 0 || length >= size)
	{
		throw dynamic_library_error(
			filename + ": " + get_system_error_message(::GetLastError())
		);
	}
	result.resize(length);
	return result;
}

} // anonymous namespace

void* dynamic_library_open(const char* filename)
{
	// A path with a directory is loaded from there, and the libraries it
	// needs are looked for beside it first; a bare name is searched for.
	const std::string name(filename);
	std::wstring path;
	DWORD flags = LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;
	if (name.find_first_of("/\\:") != std::string::npos)
	{
		path = get_full_path(name);
		flags |= LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR;
	}
	else
	{
		path = utf8_to_utf16(name);
	}

	DWORD previous_error_mode = 0;
	::SetThreadErrorMode(
		SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX,
		&previous_error_mode
	);
	const HMODULE module = ::LoadLibraryExW(path.c_str(), nullptr, flags);
	const DWORD error = ::GetLastError();
	::SetThreadErrorMode(previous_error_mode, nullptr);

	if (module == nullptr)
	{
		throw dynamic_library_error(
			name + ": " + get_system_error_message(error)
		);
	}
	return static_cast<void*>(module);
}

void dynamic_library_close(void* handle) noexcept
{
	::FreeLibrary(static_cast<HMODULE>(handle));
}

void* dynamic_library_get_symbol(void* handle, const char* name) noexcept
{
	return reinterpret_cast<void*>(
		::GetProcAddress(static_cast<HMODULE>(handle), name)
	);
}

std::string dynamic_library_symbol_filename_lookup(const void* symbol)
{
	HMODULE module = nullptr;
	const DWORD flags =
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	if (!::GetModuleHandleExW(flags, static_cast<LPCWSTR>(symbol), &module))
	{
		return std::string();
	}

	std::wstring path(MAX_PATH, L'\0');
	for (;;)
	{
		const DWORD length = ::GetModuleFileNameW(
			module, &path[0], static_cast<DWORD>(path.size())
		);
		if (length == 0)
		{
			return std::string();
		}
		if (length < path.size())
		{
			path.resize(length);
			return utf16_to_utf8(path);
		}
		path.resize(path.size() * 2);
	}
}

} // namespace rexlib
