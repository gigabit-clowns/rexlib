// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../layout/strided_layout.hpp"
#include "../numerical/numerical_type.hpp"
#include "../platform/dynamic_shared_object.h"

namespace xmipp4 
{

/**
 * @brief Description of the memory representation of a multidimensional array.
 * 
 * This class represents the (strided_layout, numerical_type) pair that
 * defines the data interpretation on a multidimensional array.
 * 
 * @see array
 * 
 */
class array_descriptor
{
public:
	/**
	 * @brief Default construct the descriptor with an empty layout and 
	 * unknown numerical type.
	 * 
	 */
	REXLIB_API
	array_descriptor() noexcept;

	/**
	 * @brief Construct a descriptor from its components.
	 * 
	 * @param layout The layout.
	 * @param data_type The numerical type.
	 */
	REXLIB_API
	array_descriptor(
		strided_layout layout,
		numerical_type data_type
	) noexcept;

	REXLIB_API
	array_descriptor(const array_descriptor &other);
	REXLIB_API
	array_descriptor(array_descriptor &&other) noexcept;
	REXLIB_API
	~array_descriptor();

	REXLIB_API
	array_descriptor& operator=(const array_descriptor &other);
	REXLIB_API
	array_descriptor& operator=(array_descriptor &&other) noexcept;

	/**
	 * @brief Get the hash value for this descriptor.
	 * 
	 * @return std::size_t The hash value.
	 */
	REXLIB_API
	std::size_t hash() const noexcept;

	/**
	 * @brief Get the layout of the array.
	 * 
	 * @return const strided_layout& The layout.
	 */
	REXLIB_API
	const strided_layout& get_layout() const noexcept;

	/**
	 * @brief Get the data type of the array.
	 * 
	 * @return numerical_type The numerical type.
	 */
	REXLIB_API
	numerical_type get_data_type() const noexcept;

	friend bool operator==(
		const array_descriptor &lhs,
		const array_descriptor &rhs
	) noexcept
	{
		return
			lhs.get_data_type() == rhs.get_data_type() &&
			lhs.get_layout() == rhs.get_layout();
	}

	friend bool operator!=(
		const array_descriptor &lhs,
		const array_descriptor &rhs
	) noexcept
	{
		return !(lhs == rhs);
	}

private:
	strided_layout m_layout;
	numerical_type m_data_type;
};

/**
 * @brief Create an array descriptor with a contiguous layout.
 *
 * The resulting descriptor holds a contiguous layout built from the given
 * extents paired with the provided numerical type.
 *
 * @param extents The extents of each dimension.
 * @param data_type The numerical type.
 * @return array_descriptor The resulting descriptor.
 *
 * @see strided_layout::make_contiguous_layout
 */
REXLIB_API
array_descriptor make_contiguous_array_descriptor(
	span<const std::size_t> extents,
	numerical_type data_type
);

/**
 * @brief Check if the array descriptor is initialized.
 * 
 * @param descriptor The descriptor to check.
 * @return true If the descriptor is initialized.
 * @return false If the descriptor is not initialized.
 */
REXLIB_API
bool is_initialized(const array_descriptor &descriptor) noexcept;

/**
 * @brief Compute the number of bytes required to store the given descriptor.
 * 
 * @param descriptor The descriptor from which the storage requirement is 
 * computed.
 * @return std::size_t Minimum number of bytes required to store the descriptor.
 */
REXLIB_API
std::size_t compute_storage_requirement(const array_descriptor &descriptor);

} // namespace xmipp4

namespace std
{

template<>
struct hash<xmipp4::array_descriptor>
{
	std::size_t operator()(
		const xmipp4::array_descriptor &descriptor
	) const noexcept
	{
		return descriptor.hash();
	}
};

} // namespace std
