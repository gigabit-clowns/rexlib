// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/operation/operand_specification.hpp>

#include <utility>

namespace rexlib
{

operand_specification::operand_specification() noexcept
	: m_shape()
	, m_data_type(numerical_type::unknown)
{
}

operand_specification::operand_specification(
	shape_type shape,
	numerical_type data_type
) noexcept
	: m_shape(std::move(shape))
	, m_data_type(data_type)
{
}

const operand_specification::shape_type&
operand_specification::get_shape() const noexcept
{
	return m_shape;
}

void operand_specification::set_shape(shape_type shape) noexcept
{
	m_shape = std::move(shape);
}

numerical_type operand_specification::get_data_type() const noexcept
{
	return m_data_type;
}

void operand_specification::set_data_type(numerical_type data_type) noexcept
{
	m_data_type = data_type;
}

operand_specification operand_specification::from_descriptor(
	const array_descriptor &descriptor
)
{
	shape_type shape;
	descriptor.get_layout().get_extents(shape);
	return operand_specification(
		std::move(shape),
		descriptor.get_data_type()
	);
}

} // namespace rexlib
