// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/system/access_flags.hpp>
#include <rex/core/platform/operating_system.h>

namespace rex
{

/**
 * @brief Memory map a file.
 * 
 * @param filename Path to the file to be mapped.
 * @param access Memory access type. It can not be empty.
 * @param size Number of bytes to be mapped. If zero, it 
 * will be written with the size of the file.
 * @param copy_on_write If true, changes are not commited 
 * to the file. Has no effect if write access is not enabled.
 * @return void* Pointer to the mapped data.
 */
void* memory_mapped_file_open(
	const char* filename, 
	access_flags access,
	std::size_t &size,
	bool copy_on_write
);

/**
 * @brief Close a memory mapped file.
 * 
 * @param data Pointer to the memory mapped data.
 * @param size Size of the memory mapped data.
 */
void memory_mapped_file_close(void* data, std::size_t size) noexcept;


} // namespace rex

#if REX_POSIX
	#include "memory_mapped_file_handle_posix.inl"
#elif REX_WINDOWS
	#include "memory_mapped_file_handle_windows.inl"
#else
	#error "No memory_mapped_file_handle implementation available for this platform"
#endif
