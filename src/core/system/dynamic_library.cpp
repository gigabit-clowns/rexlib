// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/system/dynamic_library.hpp>

#include "dynamic_library_handle.hpp"

#include <sstream>

namespace rexlib
{

dynamic_library::dynamic_library() noexcept
	: m_handle(nullptr)
{
}

dynamic_library::dynamic_library(const char* filename)
	: m_handle(dynamic_library_open(filename))
{
}

dynamic_library::dynamic_library(const std::string& filename)
	: dynamic_library(filename.c_str())
{
}

dynamic_library::dynamic_library(dynamic_library&& other) noexcept
	: m_handle(other.m_handle)
{
	other.m_handle = nullptr;
}

dynamic_library::~dynamic_library()
{
	close();
}

dynamic_library& dynamic_library::operator=(dynamic_library&& other) noexcept
{
	close();
	swap(other);
	return *this;
}

bool dynamic_library::is_open() const noexcept
{
	return m_handle != nullptr;
}

void dynamic_library::swap(dynamic_library& other) noexcept
{
	std::swap(m_handle, other.m_handle);
}

void dynamic_library::open(const char* filename)
{
	*this = dynamic_library(filename);
}

void dynamic_library::open(const std::string& filename)
{
	*this = dynamic_library(filename);
}

void dynamic_library::close() noexcept
{
	if (m_handle != nullptr)
	{
		dynamic_library_close(m_handle);
		m_handle = nullptr;
	}
}

void* dynamic_library::get_symbol(const char* name) const noexcept
{
	return dynamic_library_get_symbol(m_handle, name);
}

void* dynamic_library::get_symbol(const std::string& name) const noexcept
{
	return get_symbol(name.c_str());
}

std::string dynamic_library::query_symbol_filename(const void* symbol)
{
	return dynamic_library_symbol_filename_lookup(symbol);
}

void swap(dynamic_library& lhs, dynamic_library& rhs) noexcept
{
	lhs.swap(rhs);
}

} // namespace rexlib
