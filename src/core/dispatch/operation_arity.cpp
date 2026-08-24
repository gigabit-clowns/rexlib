// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/dispatch/operation_arity.hpp>

namespace rexlib
{

operation_arity::operation_arity() noexcept
	: operation_arity(0, 0)
{
}

operation_arity::operation_arity(
	std::size_t output_count,
	std::size_t input_count
) noexcept
	: m_output_count(output_count)
	, m_input_count(input_count)
{
}

std::size_t operation_arity::get_output_count() const noexcept
{
	return m_output_count;
}

std::size_t operation_arity::get_input_count() const noexcept
{
	return m_input_count;
}

operation_arity operation_arity::nullary() noexcept
{
	return operation_arity(1, 0);
}

operation_arity operation_arity::unary() noexcept
{
	return operation_arity(1, 1);
}

operation_arity operation_arity::binary() noexcept
{
	return operation_arity(1, 2);
}

operation_arity operation_arity::ternary() noexcept
{
	return operation_arity(1, 3);
}

} // namespace rexlib
