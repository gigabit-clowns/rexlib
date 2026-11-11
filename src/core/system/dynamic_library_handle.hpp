// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>

namespace rexlib
{

/**
 * @brief Open a dynamic library, resolving every symbol it needs.
 *
 * @param filename Path to the dynamic library, in UTF-8.
 * @return void* Handle of the newly opened dynamic library.
 * @throws dynamic_library_error If it cannot be loaded.
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
 * @brief Obtain the filename of the dynamic library from where the
 * given symbol has been loaded.
 *
 * @param symbol The symbol to be looked up.
 * @return std::string The path in UTF-8. Empty if no loaded library
 * holds the symbol.
 */
std::string dynamic_library_symbol_filename_lookup(const void* symbol);

} // namespace rexlib
