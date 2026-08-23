// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "access_flags.hpp"
#include "../platform/dynamic_shared_object.h"
#include "../platform/constexpr.hpp"

#include <type_traits>

namespace xmipp4 
{

/**
 * @brief RAII style class to memory map files
 * 
 */
class REXLIB_API memory_mapped_file
{
public:
	/**
	 * @brief Constant meaning to map the whole file
	 * 
	 */
	static REX_INLINE_CONST_CONSTEXPR std::size_t whole_file = 0;

	/**
	 * @brief Construct an empty memory-mapped file
	 * 
	 */
	memory_mapped_file() noexcept;

	/**
	 * @brief Construct with a file opened in memory.
	 * 
	 * @param filename Path to the file to be mapped.
	 * @param access Memory access permissions. Can not be empty.
	 * @param size Number of bytes to be mapped. Use whole_file (0) 
	 * to map the entire file.
	 * @param copy_on_write Do not write back changes to the file.
	 * Has no effect if write access is not enabled.
	 */
	explicit memory_mapped_file(
		const char* filename, 
		access_flags access = read_only,
		std::size_t size = whole_file,
		bool copy_on_write = false
	);

	/**
	 * @brief Construct with a file opened in memory.
	 * 
	 * @param filename Path to the file to be mapped.
	 * @param access Memory access permissions. Can not be empty.
	 * @param size Number of bytes to be mapped. Use whole_file (0) 
	 * to map the entire file.
	 * @param copy_on_write Do not write back changes to the file.
	 * Has no effect if write access is not enabled.
	 */
	explicit memory_mapped_file(
		const std::string& filename, 
		access_flags access = read_only,
		std::size_t size = whole_file,
		bool copy_on_write = false
	);
	memory_mapped_file(const memory_mapped_file& other) = delete;
	memory_mapped_file(memory_mapped_file&& other) noexcept;
	~memory_mapped_file();

	memory_mapped_file& operator=(const memory_mapped_file& other) = delete;
	memory_mapped_file& operator=(memory_mapped_file&& other) noexcept;

	/**
	 * @brief Check if there is a file opened
	 * 
	 * @return bool true if there is a file opened
	 */
	bool is_open() const noexcept;

	/**
	 * @brief Exchange the value with another instance
	 * 
	 * @param other The other instance to be exchanged with
	 * 
	 */
	void swap(memory_mapped_file& other) noexcept;

	/**
	 * @brief Opens a file in memory.
	 * In case another file is open, first it closes it.
	 * 
	 * @param filename Path to the file to be mapped.
	 * @param access Memory access permissions. Can not be empty.
	 * @param size Number of bytes to be mapped. Use whole_file (0) 
	 * to map the entire file.
	 * @param copy_on_write Do not write back changes to the file.
	 * Has no effect if write access is not enabled.
	 */
	void open(
		const char* filename, 
		access_flags access = read_only,
		std::size_t size = whole_file,
		bool copy_on_write = false
	);

	/**
	 * @brief Opens a file in memory.
	 * In case another file is open, first it closes it.
	 * 
	 * @param filename Path to the file to be mapped.
	 * @param access Memory access permissions. Can not be empty.
	 * @param size Number of bytes to be mapped. Use whole_file (0) 
	 * to map the entire file.
	 * @param copy_on_write Do not write back changes to the file.
	 * Has no effect if write access is not enabled.
	 */
	void open(
		const std::string& filename, 
		access_flags access = read_only,
		std::size_t size = whole_file,
		bool copy_on_write = false
	);

	/**
	 * @brief Close the opened file
	 * Has no effect if it is not opened
	 */
	void close() noexcept;

	/**
	 * @brief Get the size of the memory mapped area
	 * 
	 * @return std::size_t Size in bytes
	 */
	std::size_t size() const noexcept;

	/**
	 * @brief Get a pointer to the memory mapped area
	 * 
	 * @return void* Pointer to the data
	 */
	void* data() const noexcept;

private:
	std::size_t m_size;
	void* m_data;
};

/**
 * @brief Exchange two memory_mapped_file objects
 *
 */
REXLIB_API void swap(memory_mapped_file& lhs, memory_mapped_file& rhs) noexcept;

} //namespace xmipp4
