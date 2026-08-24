// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/dynamic_shared_object.h"

#include <memory>

namespace rexlib 
{

class buffer;


class array;
class array_descriptor;
class array_implementation;

/**
 * @brief Read-only view into an array.
 * 
 * A const_array shares ownership of the implementation with another array,
 * but it only allows read-only semantics with it.
 * 
 * @see array
 */
class const_array
{
public:	
	/**
	 * @brief Construct an uninitialized array view.
	 */
	REXLIB_API const_array();

	/**
	 * @brief Construct a const_array from a shared pointer to the 
	 * implementation.
	 * 
	 * @param implementation The implementation to be owned by this.
	 * 
	 * @note This function is not part of the public API and shall only be
	 * used internally or for testing purposes.
	 */
	explicit const_array(
		std::shared_ptr<const array_implementation> implementation
	) noexcept;

	const_array(const const_array& other) = delete;
	REXLIB_API const_array(const_array&& other) noexcept;
	REXLIB_API ~const_array();

	const_array& operator=(const const_array& other) = delete;
	REXLIB_API const_array& operator=(const_array&& other) noexcept;

	/**
	 * @brief Get the descriptor of this array.
	 * 
	 * @return array_descriptor The descriptor.
	 */
	REXLIB_API const array_descriptor& get_descriptor() const noexcept;

	/**
	 * @brief Get the storage of this array.
	 * 
	 * @return const buffer* The storage. nullptr if the array is not
	 * initialized
	 */
	REXLIB_API const buffer* get_storage() const noexcept;
	
	/**
	 * @brief Get the storage of this const_array.
	 * 
	 * @return std::shared_ptr<const buffer> The storage. nullptr if 
	 * the const_array is not initialized
	 */
	REXLIB_API 
	std::shared_ptr<const buffer> share_storage() const noexcept;

	/**
	 * @brief Obtain an alias for this array.
	 * 
	 * @return const_array Another const_array referencing the contents of this 
	 * array.
	 */
	REXLIB_API
	const_array share() const noexcept;

	/**
	 * @brief Get a pointer to the implementation.
	 *
	 * @return const array_implementation* Pointer to the implementation.
	 * nullptr if the const_array is not initialized.
	 *
	 * @note This method is meant to be used internally and for testing
	 * purposes, as the array_implementation is not publicly available.
	 */
	const array_implementation* get_implementation() const noexcept;

private:
	std::shared_ptr<const array_implementation> m_implementation;
};

} // namespace rexlib
