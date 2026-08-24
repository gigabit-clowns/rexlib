// SPDX-License-Identifier: GPL-3.0-only

#include "memory_mapped_file_handle.hpp"

#include <rex/core/platform/constexpr.hpp>

#include <stdexcept>
#include <system_error>
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace rexlib
{

inline int access_flags_to_open_flags(
	access_flags access,
	bool copy_on_write
)
{
	int result = 0;

	if(copy_on_write)
	{
		result = O_RDONLY;
	}
	else
	{
		if (access == read_write) 
		{
			result = O_RDWR;
		}
		else if (access == read_only) 
		{
			result = O_RDONLY;
		}
		else if (access == write_only) 
		{
			result = O_WRONLY;
		}
		else 
		{
			throw std::invalid_argument("Unsupported access");
		}
	}

	return result;
}

inline int access_flags_to_mmap_prot_flags(access_flags access) noexcept
{
	int prot = PROT_NONE;

	if (access.contains(access_flag_bits::read)) 
	{
		prot |= PROT_READ;
	}
	if (access.contains(access_flag_bits::write)) 
	{
		prot |= PROT_WRITE;
	}

	return prot;
}

inline int open_file(
	const char* filename, 
	access_flags access,
	bool copy_on_write
)
{
	const auto open_flags = access_flags_to_open_flags(access, copy_on_write);
	const auto file = open(filename, open_flags);

	if (file < 0) 
	{
		throw std::system_error(
			errno, std::generic_category(), "Error opening file"
		);
	}
	return file;
}

inline void* memory_map_file_descriptor(
	int file, 
	access_flags access, 
	std::size_t size,
	bool copy_on_write
)
{
	const int prot = access_flags_to_mmap_prot_flags(access); 
	const int flags = copy_on_write ? MAP_PRIVATE : MAP_SHARED;
	REXLIB_CONST_CONSTEXPR off_t offset = 0;

	auto *const result = mmap(
		nullptr,
		size,
		prot,
		flags,
		file,
		offset
	);

	if (result == MAP_FAILED)
	{
		throw std::system_error(
			errno, std::generic_category(), "Error memory mapping the file"
		);
	}

	return result;
}

inline std::size_t get_file_size(int file) noexcept
{
	struct stat file_stats = {};
	fstat(file, &file_stats);
	return static_cast<std::size_t>(file_stats.st_size);
}

inline void* memory_mapped_file_open(
	const char* filename, 
	access_flags access,
	std::size_t &size,
	bool copy_on_write
)
{
	const auto file = open_file(
		filename, 
		access,
		copy_on_write
	);

	void* result = nullptr;
	try
	{
		if (size == 0) 
		{
			size = get_file_size(file);
		}

		result = memory_map_file_descriptor(
			file, 
			access, 
			size, 
			copy_on_write
		);
	}
	catch(...)
	{
		close(file);
		throw;
	}
	
	// Close the file descriptor as it is not 
	// longer needed
	close(file);

	return result;
}

inline void memory_mapped_file_close(void* data, std::size_t size) noexcept
{
	munmap(data, size);
}

} // namespace rexlib
