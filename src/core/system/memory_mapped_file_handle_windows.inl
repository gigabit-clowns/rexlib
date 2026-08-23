// SPDX-License-Identifier: GPL-3.0-only

#include "memory_mapped_file_handle.hpp"

#include <xmipp4/core/platform/constexpr.hpp>

#include <stdexcept>
#include <sstream>
#include <windows.h>

namespace rex
{

inline DWORD access_flags_to_open_access(
	access_flags access,
	bool copy_on_write
)
{
	DWORD result = 0;
	
	if(access.contains(access_flag_bits::read)) 
		result |= GENERIC_READ;
	if(access.contains(access_flag_bits::write) && !copy_on_write) 
		result |= GENERIC_WRITE;

	return result;
}

inline DWORD access_flags_to_memory_map_protect(
	access_flags access,
	bool copy_on_write
)
{
	DWORD result;

	if (access.contains(access_flag_bits::write))
	{
		if (copy_on_write)
			result = PAGE_WRITECOPY;
		else
			result = PAGE_READWRITE;
	}
	else if (access.contains(access_flag_bits::read))
	{
		result = PAGE_READONLY;
	}
	else 
	{
		throw std::invalid_argument("Unsupported access");
	}

	return result;
}

inline DWORD access_flags_to_view_access(
	access_flags access,
	bool copy_on_write
)
{
	DWORD result;

	if(copy_on_write && access.contains(access_flag_bits::write))
	{
		result = FILE_MAP_COPY;
	}
	else
	{
		result = 0;
		if(access.contains(access_flag_bits::read)) 
			result |= FILE_MAP_READ;
		if(access.contains(access_flag_bits::write)) 
			result |= FILE_MAP_WRITE;
	}

	return result;
}

inline HANDLE open_file(
	const char* filename, 
	access_flags access,
	bool copy_on_write
)
{
	const DWORD desired_access = access_flags_to_open_access(access, copy_on_write);
	REX_CONST_CONSTEXPR DWORD share_mode = 0;
	REX_CONST_CONSTEXPR LPSECURITY_ATTRIBUTES security_attributes = 0;
	REX_CONST_CONSTEXPR DWORD create_mode = OPEN_EXISTING;
	REX_CONST_CONSTEXPR DWORD flags = FILE_ATTRIBUTE_NORMAL;
	REX_CONST_CONSTEXPR HANDLE template_file = 0;

	HANDLE result = CreateFileA(
		filename,
		desired_access,
		share_mode,
		security_attributes,
		create_mode,
		flags,
		template_file
	);

	if(result == INVALID_HANDLE_VALUE)
	{
		std::stringstream oss;
		oss << "Could not open file " << filename << " Error " << GetLastError();
		throw std::runtime_error(oss.str());
	}

	return result;
}

inline HANDLE create_file_mapping(
	HANDLE file, 
	access_flags access,
	bool copy_on_write
)
{
	const DWORD protect = access_flags_to_memory_map_protect(access, copy_on_write);
	REX_CONST_CONSTEXPR DWORD maximum_size_low = 0;
	REX_CONST_CONSTEXPR DWORD maximum_size_high = 0;
	REX_CONST_CONSTEXPR LPSECURITY_ATTRIBUTES security_attributes = 0;
	REX_CONST_CONSTEXPR LPCSTR name = 0;

	HANDLE result = CreateFileMappingA(
		file, 
		security_attributes,
		protect,
		maximum_size_high,
		maximum_size_low,
		name
	);

	if(result == INVALID_HANDLE_VALUE)
	{
		std::stringstream oss;
		oss << "Could not create file mapping. Error " << GetLastError();
		throw std::runtime_error(oss.str());
	}

	return result;
}

inline HANDLE create_file_mapping_view(
	HANDLE mapping, 
	access_flags access,
	std::size_t size,
	bool copy_on_write
)
{
	const DWORD desired_access = access_flags_to_view_access(access, copy_on_write);
	REX_CONST_CONSTEXPR DWORD offset_high = 0;
	REX_CONST_CONSTEXPR DWORD offset_low = 0;

	void* result = MapViewOfFile(
		mapping,
		desired_access,
		offset_high,
		offset_low,
		size
	);

	if(result == NULL)
	{
		std::stringstream oss;
		oss << "Could not create file mapping view. Error " << GetLastError();
		throw std::runtime_error(oss.str());
	}

	return result;
}

inline std::size_t get_file_size(HANDLE handle)
{
	LARGE_INTEGER size;
	if(GetFileSizeEx(handle, &size) == 0)
	{
		throw std::runtime_error("Could not retrieve file size");
	}
	return size.QuadPart;
}

inline void* memory_mapped_file_open(
	const char* filename, 
	access_flags access,
	std::size_t &size,
	bool copy_on_write
)
{
	const auto file_handle = open_file(
		filename, 
		access,
		copy_on_write
	);
	
	// Create a file mapping for the file
	HANDLE mapping_handle;
	try
	{
		if(size == 0) size = get_file_size(file_handle);
		mapping_handle = create_file_mapping(
			file_handle, 
			access, 
			copy_on_write
		);
	}
	catch(...)
	{
		CloseHandle(file_handle);
		throw;
	}

	// Create a file mapping for the file
	void* result;
	try
	{
		result = create_file_mapping_view(
			mapping_handle, 
			access, 
			size, 
			copy_on_write
		);
	}
	catch(...)
	{
		CloseHandle(mapping_handle);
		CloseHandle(file_handle);
		throw;
	}

	// Close intermediate handles
	// As stated in the documentation, this is safe, as they are
	// reference counted and the view holds a reference to them.
	CloseHandle(mapping_handle);
	CloseHandle(file_handle);

	return result;
}

inline void memory_mapped_file_close(void* data, std::size_t) noexcept
{
	UnmapViewOfFile(data);
}

} // namespace rex
