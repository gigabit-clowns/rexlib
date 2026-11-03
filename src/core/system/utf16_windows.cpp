// SPDX-License-Identifier: GPL-3.0-only

#include "utf16_windows.hpp"

#include <windows.h>

#include <limits>
#include <stdexcept>

namespace rexlib
{

namespace
{

int get_length(std::size_t size)
{
	if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
	{
		throw std::invalid_argument("The text is too long to convert.");
	}
	return static_cast<int>(size);
}

} // anonymous namespace

std::wstring utf8_to_utf16(const std::string& utf8)
{
	if (utf8.empty())
	{
		return std::wstring();
	}

	const int length = get_length(utf8.size());
	const int size = ::MultiByteToWideChar(
		CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), length, nullptr, 0
	);
	if (size == 0)
	{
		throw std::invalid_argument("The text is not valid UTF-8.");
	}

	std::wstring result(static_cast<std::size_t>(size), L'\0');
	::MultiByteToWideChar(
		CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), length, &result[0], size
	);
	return result;
}

std::string utf16_to_utf8(const std::wstring& utf16)
{
	if (utf16.empty())
	{
		return std::string();
	}

	const int length = get_length(utf16.size());
	const int size = ::WideCharToMultiByte(
		CP_UTF8, 0, utf16.data(), length, nullptr, 0, nullptr, nullptr
	);

	std::string result(static_cast<std::size_t>(size), '\0');
	::WideCharToMultiByte(
		CP_UTF8, 0, utf16.data(), length, &result[0], size, nullptr, nullptr
	);
	return result;
}

} // namespace rexlib
