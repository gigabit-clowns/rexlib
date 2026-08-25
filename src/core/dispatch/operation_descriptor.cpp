// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/dispatch/operation_descriptor.hpp>

#include <string>

namespace rexlib
{

operation_descriptor::operation_descriptor(
	const char *component,
	const char *name,
	span<const char* const> output_operand_names,
	span<const char* const> input_operand_names
) noexcept
	: m_component(component)
	, m_name(name)
	, m_output_operand_names(output_operand_names)
	, m_input_operand_names(input_operand_names)
{
}

const char* operation_descriptor::get_component() const noexcept
{
	return m_component;
}

const char* operation_descriptor::get_name() const noexcept
{
	return m_name;
}

operation_arity operation_descriptor::get_arity() const noexcept
{
	return operation_arity(
		m_output_operand_names.size(),
		m_input_operand_names.size()
	);
}

span<const char* const>
operation_descriptor::get_output_operand_names() const noexcept
{
	return m_output_operand_names;
}

span<const char* const>
operation_descriptor::get_input_operand_names() const noexcept
{
	return m_input_operand_names;
}

const char* operation_descriptor::get_operand_name(
	std::size_t index,
	bool output_operand
) const noexcept
{
	const auto &names =
		output_operand ? m_output_operand_names : m_input_operand_names;

	return index < names.size() ? names[index] : nullptr;
}

std::string describe_operand(
	const operation_descriptor &descriptor,
	std::size_t index,
	bool output_operand
)
{
	const auto *name = descriptor.get_operand_name(index, output_operand);
	if (name != nullptr)
	{
		return "'" + std::string(name) + "'";
	}

	return std::to_string(index);
}

} // namespace rexlib
