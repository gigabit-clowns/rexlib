// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "array_ref.hpp"
#include "../platform/dynamic_shared_object.h"

#include <memory>

namespace rex
{

class buffer;


class array;
class array_descriptor;
class array_implementation;
class const_array;

/**
 * @brief Non-owning read-only reference to an array.
 *
 * A const_array_ref refers to the contents of an array without sharing
 * its ownership and only allows read-only semantics with it. It is
 * intended to be used as a parameter type at interface boundaries where
 * ownership does not need to be extended, analogously to std::string_view. 
 * The referenced array must outlive the const_array_ref; do not store 
 * const_array_ref objects.
 *
 * @see const_array
 * @see array_ref
 */
class const_array_ref
{
public:
	/**
	 * @brief Construct an uninitialized const_array_ref.
	 */
	REXLIB_API
	const_array_ref() noexcept;

	/**
	 * @brief Construct a const_array_ref from an implementation pointer.
	 * 
	 * @param implementation The implementation referenced by this.
	 * 
	 * @note This function is not part of the public API and shall only be
	 * used internally or for testing purposes.
	 */
	explicit const_array_ref(const array_implementation *implementation) noexcept;

	/**
	 * @brief Construct a const_array_ref referencing an array.
	 *
	 * @param other The array to be referenced. Must outlive this
	 * const_array_ref.
	 */
	REXLIB_API
	const_array_ref(const array &other) noexcept;

	/**
	 * @brief Construct a const_array_ref referencing a const_array.
	 *
	 * @param other The const_array to be referenced. Must outlive this
	 * const_array_ref.
	 */
	REXLIB_API
	const_array_ref(const const_array &other) noexcept;

	/**
	 * @brief Construct a const_array_ref from an array_ref.
	 *
	 * @param other The array_ref whose referenced array is to be
	 * referenced.
	 */
	REXLIB_API
	const_array_ref(array_ref other) noexcept;

	const_array_ref(const const_array_ref& other) noexcept = default;
	const_array_ref(const_array_ref&& other) noexcept = default;
	~const_array_ref() = default;

	const_array_ref& operator=(const const_array_ref& other) noexcept = default;
	const_array_ref& operator=(const_array_ref&& other) noexcept = default;

	/**
	 * @brief Get the descriptor of this array.
	 *
	 * @return array_descriptor The descriptor.
	 */
	REXLIB_API const array_descriptor& get_descriptor() const noexcept;

	/**
	 * @brief Get the storage of this array.
	 *
	 * @return const buffer* The storage. nullptr if the const_array_ref
	 * is not initialized.
	 */
	REXLIB_API const buffer* get_storage() const noexcept;

	/**
	 * @brief Get a shared pointer to the storage of this array.
	 *
	 * @return std::shared_ptr<const buffer> The storage. nullptr if
	 * the const_array_ref is not initialized.
	 */
	REXLIB_API
	std::shared_ptr<const buffer> share_storage() const noexcept;

	/**
	 * @brief Get a pointer to the implementation.
	 *
	 * @return const array_implementation* Pointer to the implementation.
	 * nullptr if the const_array_ref is not initialized.
	 *
	 * @note This method is meant to be used internally and for testing
	 * purposes, as the array_implementation is not publicly available.
	 */
	const array_implementation* get_implementation() const noexcept;

private:
	const array_implementation *m_implementation;
};

} // namespace rex
