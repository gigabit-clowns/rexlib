// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/ndarray/array_descriptor.hpp>

#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{

class array;
class const_array_ref;


/**
 * @brief Logical representation of an operand's properties.
 *
 * This class contains the memory representation of the array data
 * (`array_descriptor`) and its placement in memory (pointer to a
 * `memory_resource`)
 *
 * @see array
 * @see array_descriptor
 * @see memory_resource
 */
class operand_descriptor
{
public:
	REXLIB_API
	operand_descriptor() noexcept;

	REXLIB_API
	explicit operand_descriptor(
		const array_descriptor &descriptor,
		const memory_resource* resource = nullptr
	) noexcept;

	REXLIB_API
	explicit operand_descriptor(
		array_descriptor &&descriptor,
		const memory_resource* resource = nullptr
	) noexcept;

	REXLIB_API
	operand_descriptor(
		const strided_layout& layout,
		numerical_type data_type,
		const memory_resource* resource = nullptr
	) noexcept;

	REXLIB_API
	operand_descriptor(
		strided_layout&& layout,
		numerical_type data_type,
		const memory_resource* resource = nullptr
	) noexcept;

	REXLIB_API
	operand_descriptor(const operand_descriptor &other);
	REXLIB_API
	operand_descriptor(operand_descriptor &&other) noexcept;
	REXLIB_API
	~operand_descriptor();

	REXLIB_API
	operand_descriptor& operator=(const operand_descriptor &other);
	REXLIB_API
	operand_descriptor& operator=(operand_descriptor &&other) noexcept;

	friend bool operator==(
		const operand_descriptor &lhs,
		const operand_descriptor &rhs
	) noexcept
	{
		return
			lhs.get_array_descriptor() == rhs.get_array_descriptor() &&
			lhs.get_memory_resource() == rhs.get_memory_resource();
	}

	friend bool operator!=(
		const operand_descriptor &lhs,
		const operand_descriptor &rhs
	) noexcept
	{
		return !(lhs == rhs);
	}

	/**
	 * @brief Get the hash value for this object.
	 *
	 * @return The hash value.
	 */
	REXLIB_API
	std::size_t hash() const noexcept;

	/**
	 * @brief Get the in memory layout of the data.
	 *
	 * @return const strided_layout& The layout.
	 */
	REXLIB_API
	const strided_layout& get_layout() const noexcept;

	/**
	 * @brief Get the data type of the elements.
	 *
	 * @return numerical_type The data type.
	 */
	REXLIB_API
	numerical_type get_data_type() const noexcept;

	/**
	 * @brief Get the array descriptor.
	 *
	 * @return const array_descriptor& The array descriptor.
	 */
	REXLIB_API
	const array_descriptor& get_array_descriptor() const noexcept;

	/**
	 * @brief Set the array descriptor.
	 *
	 * @param descriptor The new array descriptor.
	 */
	REXLIB_API
	void set_array_descriptor(
		const array_descriptor &descriptor
	) noexcept;

	/**
	 * @brief Get the memory resource.
	 *
	 * @return const memory_resource* The memory resource.
	 */
	REXLIB_API
	const memory_resource* get_memory_resource() const noexcept;

	/**
	 * @brief Set the memory resource.
	 *
	 * @param resource The new memory resource.
	 */
	REXLIB_API
	void set_memory_resource(
		const memory_resource* resource
	) noexcept;

	/**
	 * @brief Construct an operand descriptor from an existing array.
	 *
	 * @param a Reference to the array from which the descriptor is
	 * deduced.
	 * @return operand_descriptor The newly constructed operand descriptor.
	 */
	REXLIB_API
	static operand_descriptor from_array(const_array_ref a) noexcept;

private:
	array_descriptor m_descriptor;
	const memory_resource* m_memory_region;
};

} // namespace rexlib
