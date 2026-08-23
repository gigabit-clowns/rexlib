// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "array_descriptor.hpp"
#include "const_array.hpp"
#include "../platform/dynamic_shared_object.h"

#include <memory>

namespace xmipp4 
{

class buffer;


class array_implementation;

/**
 * @brief Container for multidimensional numeric data.
 * 
 * This class allows to represent multidimensional numeric data in a hardware
 * agnostic manner. It serves as a foundational basis to define interfaces that
 * operate in such date. The array is represented as the (storage, descriptor)
 * pair. The storage refers to an untyped region of memory (which may reside in 
 * accelerator hardware like GPUs). The descriptor defines the interpretation of
 * that data to give raise to the notion of dimensions.
 */
class array
{
public:
	/**
	 * @brief Construct an uninitialized array.
	 */
	REXLIB_API array();

	/**
	 * @brief Construct an array from a shared pointer to the implementation.
	 * 
	 * @param implementation The implementation to be owned by this.
	 * 
	 * @note This function is not part of the public API and shall only be
	 * used internally or for testing purposes.
	 */
	explicit array(
		std::shared_ptr<const array_implementation> implementation
	) noexcept;

	/**
	 * @brief Construct an array from its components.
	 * 
	 * @param storage Buffer used to store the data contained in the array.
	 * @param descriptor The interpretation of the buffer memory.
	 */
	REXLIB_API array(
		std::shared_ptr<buffer> storage,
		array_descriptor descriptor
	);

	array(const array& other) = delete;
	REXLIB_API array(array&& other) noexcept;
	REXLIB_API ~array();

	array& operator=(const array& other) = delete;
	REXLIB_API array& operator=(array&& other) noexcept;

	/**
	 * @brief Get the descriptor of this array.
	 * 
	 * @return array_descriptor The descriptor.
	 */
	REXLIB_API const array_descriptor& get_descriptor() const noexcept;

	/**
	 * @brief Get the storage of this array.
	 * 
	 * @return buffer* The storage. nullptr if the array is not 
	 * initialized.
	 */
	REXLIB_API buffer* get_storage() noexcept;

	/**
	 * @brief Get the storage of this array.
	 * 
	 * @return const buffer* The storage. nullptr if the array is not
	 * initialized
	 */
	REXLIB_API const buffer* get_storage() const noexcept;
	
	/**
	 * @brief Get a shared pointer to the storage of this array.
	 * 
	 * @return std::shared_ptr<buffer> The storage. nullptr if the 
	 * array is not initialized.
	 */
	REXLIB_API 
	std::shared_ptr<buffer> share_storage() noexcept;
	
	/**
	 * @brief Get the storage of this array.
	 * 
	 * @return std::shared_ptr<const buffer> The storage. nullptr if 
	 * the array is not initialized
	 */
	REXLIB_API 
	std::shared_ptr<const buffer> share_storage() const noexcept;

	/**
	 * @brief Obtain an alias for this array.
	 * 
	 * @return array Another array referencing the contents of this array.
	 */
	REXLIB_API 
	array share() noexcept;

	/**
	 * @brief Obtain a read-only alias for this array
	 *
	 * @return const_array A const_array referencing the contents of this
	 * array.
	 */
	REXLIB_API
	const_array share() const noexcept;

	/**
	 * @brief Obtain a read-only alias for this array.
	 *
	 * Equivalent to the const overload of share(). The distinct name makes
	 * the read-only result explicit when called on a mutable array.
	 *
	 * @return const_array A const_array referencing the contents of this
	 * array.
	 */
	REXLIB_API
	const_array share_const() const noexcept;

	/**
	 * @brief Get a pointer to the implementation.
	 *
	 * @return const array_implementation* Pointer to the implementation.
	 * nullptr if the array is not initialized.
	 *
	 * @note This method is meant to be used internally and for testing
	 * purposes, as the array_implementation is not publicly available.
	 */
	const array_implementation* get_implementation() const noexcept;

private:
	std::shared_ptr<const array_implementation> m_implementation;
};

} // namespace xmipp4
