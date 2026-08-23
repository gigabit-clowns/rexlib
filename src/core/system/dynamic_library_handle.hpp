// SPDX-License-Identifier: GPL-3.0-only

#pragma once


#include <xmipp4/core/platform/operating_system.h>

#include <string>

namespace xmipp4
{

/**
 * @brief Open a the dynamic library
 * 
 * @param filename Path to the dynamic library
 * @return void* Pointer to the newly opened dynamic library
 */
void* dynamic_library_open(const char* filename);

/**
 * @brief Close a dynamic library
 * 
 * @param handle Handle of the dynamic library to be closed
 */
void dynamic_library_close(void* handle) noexcept;

/**
 * @brief Get a symbol from the dynamic library
 * 
 * @param handle Handle of the dynamic library
 * @param name Name of the symbol
 * @return void* Pointer to the queried symbol. NULL if not found
 */
void* dynamic_library_get_symbol(void* handle, const char* name) noexcept;

/**
 * @brief Obtain the filename to the dynamic library from where the
 * given symbol has been loaded.
 * 
 * @param symbol 
 * @return std::string 
 */
std::string dynamic_library_symbol_filename_lookup(const void* symbol);

} // namespace xmipp4

#if REX_POSIX
	#include "dynamic_library_handle_posix.inl"
#elif REX_WINDOWS
	#include "dynamic_library_handle_windows.inl"
#else
	#error "No dynamic library_handle implementation available for this platform"
#endif