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
 * @brief Non-owning reference to an array.
 *
 * An array_ref refers to the contents of an array without sharing its
 * ownership. It is intended to be used as a parameter type at interface
 * boundaries where ownership does not need to be extended, analogously
 * to std::string_view. The referenced array must outlive the array_ref;
 * do not store array_ref objects.
 *
 * @see array
 * @see const_array_ref
 */
class array_ref
{
public:
	/**
	 * @brief Construct an uninitialized array_ref.
	 */
	REXLIB_API
	array_ref() noexcept;

	/**
	 * @brief Construct an array_ref from an implementation pointer.
	 * 
	 * @param implementation The implementation referenced by this.
	 * 
	 * @note This function is not part of the public API and shall only be
	 * used internally or for testing purposes.
	 */
	explicit array_ref(const array_implementation *implementation) noexcept;

	/**
	 * @brief Construct an array_ref referencing an array.
	 *
	 * @param other The array to be referenced. Must outlive this
	 * array_ref.
	 */
	REXLIB_API
	array_ref(array &other) noexcept;

	array_ref(const array_ref& other) noexcept = default;
	array_ref(array_ref&& other) noexcept = default;
	~array_ref() = default;

	array_ref& operator=(const array_ref& other) noexcept = default;
	array_ref& operator=(array_ref&& other) noexcept = default;

	/**
	 * @brief Get the descriptor of this array.
	 *
	 * @return array_descriptor The descriptor.
	 */
	REXLIB_API const array_descriptor& get_descriptor() const noexcept;

	/**
	 * @brief Get the storage of this array.
	 *
	 * @return buffer* The storage. nullptr if the array_ref is not
	 * initialized.
	 */
	REXLIB_API buffer* get_storage() noexcept;

	/**
	 * @brief Get the storage of this array.
	 *
	 * @return const buffer* The storage. nullptr if the array_ref is not
	 * initialized.
	 */
	REXLIB_API const buffer* get_storage() const noexcept;

	/**
	 * @brief Get a shared pointer to the storage of this array.
	 *
	 * @return std::shared_ptr<buffer> The storage. nullptr if the
	 * array_ref is not initialized.
	 */
	REXLIB_API
	std::shared_ptr<buffer> share_storage() noexcept;

	/**
	 * @brief Get a shared pointer to the storage of this array.
	 *
	 * @return std::shared_ptr<const buffer> The storage. nullptr if
	 * the array_ref is not initialized.
	 */
	REXLIB_API
	std::shared_ptr<const buffer> share_storage() const noexcept;

	/**
	 * @brief Get a pointer to the implementation.
	 *
	 * @return const array_implementation* Pointer to the implementation.
	 * nullptr if the array_ref is not initialized.
	 *
	 * @note This method is meant to be used internally and for testing
	 * purposes, as the array_implementation is not publicly available.
	 */
	const array_implementation* get_implementation() const noexcept;

private:
	const array_implementation *m_implementation;
};

} // namespace rexlib
