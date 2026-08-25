// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../version.hpp"
#include "../platform/dynamic_shared_object.h"

#include <string>
#include <cstdint>

namespace rexlib
{

/**
 * @brief RAII style class to load, query and unload
 * dynamic libraries
 * 
 */
class REXLIB_API dynamic_library
{
public:
	/**
	 * @brief Construct an empty dynamic_library object
	 * 
	 */
	dynamic_library() noexcept;

	/**
	 * @brief Construct a new dynamic_library object from a file
	 * 
	 * @param filename Path of the dynamic library file
	 */
	explicit dynamic_library(const char* filename);

	/**
	 * @brief Construct a new dynamic_library object from a file
	 * 
	 * @param filename Path of the dynamic library file
	 */
	explicit dynamic_library(const std::string& filename);

	dynamic_library(const dynamic_library& other) = delete;
	dynamic_library(dynamic_library&& other) noexcept;
	~dynamic_library();

	dynamic_library& operator=(const dynamic_library& other) = delete;
	dynamic_library& operator=(dynamic_library&& other) noexcept;

	void swap(dynamic_library& other) noexcept;

	/**
	 * @brief Check if a dynamic library is loaded
	 * 
	 * @return bool Returns true if a library is loaded
	 */
	bool is_open() const noexcept;

	/**
	 * @brief Loads a dynamic library
	 * 
	 * @param filename Path to the dynamic library
	 */
	void open(const char* filename);

	/**
	 * @brief Loads a dynamic library
	 * 
	 * @param filename Path to the dynamic library
	 */
	void open(const std::string& filename);

	/**
	 * @brief Frees the stored dynamic library, if any
	 * 
	 */
	void close() noexcept;

	/**
	 * @brief Get the address of a symbol
	 * 
	 * @param name Name of the symbol
	 * @return void* Pointer to the symbol. nullptr if not found
	 */
	void* get_symbol(const char* name) const noexcept; 

	/**
	 * @brief Get the address of a symbol
	 * 
	 * @param name Name of the symbol
	 * @return void* Pointer to the symbol. nullptr if not found
	 */
	void* get_symbol(const std::string& name) const noexcept; 

	/**
	 * @brief Obtain the filename to the dynamic library from where the
	 * given symbol has been loaded.
	 * 
	 * @param symbol The symbol to be looked up.
	 * @return std::string The path to the dynamic library from where 
	 * the symbol has been loaded. Empty if no matching symbol was found. 
	 */
	static std::string query_symbol_filename(const void* symbol);

private:
	void* m_handle;
};

/**
 * @brief Exchange two dynamic_library objects
 */
REXLIB_API void swap(dynamic_library& lhs, dynamic_library& rhs) noexcept;

} // namespace rexlib
