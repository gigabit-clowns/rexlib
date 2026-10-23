// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../ndarray/array_descriptor.hpp"
#include "../numerical/numerical_type.hpp"
#include "../platform/dynamic_shared_object.h"

#include <cstddef>
#include <vector>

namespace rexlib
{

/**
 * @brief Shape and data type of an operand.
 *
 * Describes an operand apart from where and how its elements are stored, so
 * it holds neither strides nor an offset.
 *
 * @see operation_contract
 */
class operand_specification
{
public:
	using shape_type = std::vector<std::size_t>;

	/**
	 * @brief Construct a specification with no axes and an unknown type.
	 */
	REXLIB_API
	operand_specification() noexcept;

	/**
	 * @brief Construct a specification.
	 *
	 * @param shape The extent of every axis.
	 * @param data_type The type of the elements.
	 */
	REXLIB_API
	operand_specification(
		shape_type shape,
		numerical_type data_type
	) noexcept;

	operand_specification(const operand_specification &other) = default;
	operand_specification(operand_specification &&other) noexcept = default;
	~operand_specification() = default;

	operand_specification&
	operator=(const operand_specification &other) = default;
	operand_specification&
	operator=(operand_specification &&other) noexcept = default;

	/**
	 * @brief Get the extent of every axis.
	 *
	 * @return const shape_type& The shape.
	 */
	REXLIB_API
	const shape_type& get_shape() const noexcept;

	/**
	 * @brief Set the extent of every axis.
	 *
	 * @param shape The new shape.
	 */
	REXLIB_API
	void set_shape(shape_type shape) noexcept;

	/**
	 * @brief Get the type of the elements.
	 *
	 * @return numerical_type The data type.
	 */
	REXLIB_API
	numerical_type get_data_type() const noexcept;

	/**
	 * @brief Set the type of the elements.
	 *
	 * @param data_type The new data type.
	 */
	REXLIB_API
	void set_data_type(numerical_type data_type) noexcept;

	/**
	 * @brief Get the specification of an operand with a given descriptor.
	 *
	 * @param descriptor The descriptor of the operand.
	 * @return operand_specification The extents and the data type of
	 * @p descriptor.
	 */
	REXLIB_API
	static operand_specification from_descriptor(
		const array_descriptor &descriptor
	);

	friend bool operator==(
		const operand_specification &lhs,
		const operand_specification &rhs
	) noexcept
	{
		return
			lhs.get_shape() == rhs.get_shape() &&
			lhs.get_data_type() == rhs.get_data_type();
	}

	friend bool operator!=(
		const operand_specification &lhs,
		const operand_specification &rhs
	) noexcept
	{
		return !(lhs == rhs);
	}

private:
	shape_type m_shape;
	numerical_type m_data_type;
};

} // namespace rexlib
