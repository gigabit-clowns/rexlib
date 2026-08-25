// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/dispatch/rules/operand_type_resolution.hpp>

#include <rexlib/core/platform/assert.hpp>

#include <algorithm>

namespace rexlib
{

const char* to_string(type_rule_status status) noexcept
{
	switch (status)
	{
	case type_rule_status::ok:
		return "ok";
	case type_rule_status::invalid_type:
		return "invalid type";
	case type_rule_status::domain_violation:
		return "domain violation";
	case type_rule_status::undefined_transform:
		return "undefined transform";
	case type_rule_status::slot_mismatch:
		return "slot mismatch";
	case type_rule_status::unresolved_pivot:
		return "unresolved pivot";
	}

	return "unknown";
}

type_rule_resolution::type_rule_resolution() noexcept
	: m_pivot_count(0)
	, m_status(type_rule_status::ok)
	, m_operand_index(0)
	, m_output_operand(false)
	, m_offending_type(numerical_type::unknown)
{
	std::fill(m_pivots.begin(), m_pivots.end(), numerical_type::unknown);
}

type_rule_resolution type_rule_resolution::failure(
	type_rule_status status,
	std::size_t operand_index,
	bool output_operand,
	numerical_type type
) noexcept
{
	REXLIB_ASSERT(status != type_rule_status::ok);

	type_rule_resolution result;
	result.m_status = status;
	result.m_operand_index = operand_index;
	result.m_output_operand = output_operand;
	result.m_offending_type = type;
	return result;
}

type_rule_resolution type_rule_resolution::fail(
	type_rule_status status,
	std::size_t operand_index,
	bool output_operand,
	numerical_type type
) const noexcept
{
	REXLIB_ASSERT(status != type_rule_status::ok);

	type_rule_resolution result(*this);
	result.m_status = status;
	result.m_operand_index = operand_index;
	result.m_output_operand = output_operand;
	result.m_offending_type = type;
	return result;
}

type_rule_status type_rule_resolution::get_status() const noexcept
{
	return m_status;
}

type_rule_resolution::operator bool() const noexcept
{
	return m_status == type_rule_status::ok;
}

std::size_t type_rule_resolution::get_pivot_count() const noexcept
{
	return m_pivot_count;
}

numerical_type type_rule_resolution::get_pivot(
	std::size_t index
) const noexcept
{
	REXLIB_ASSERT(index < m_pivot_count);
	return m_pivots[index];
}

void type_rule_resolution::set_pivot(
	std::size_t index,
	numerical_type type
) noexcept
{
	REXLIB_ASSERT(index < max_operand_type_pivot_count);
	m_pivots[index] = type;
}

void type_rule_resolution::set_pivot_count(std::size_t count) noexcept
{
	REXLIB_ASSERT(count <= max_operand_type_pivot_count);
	m_pivot_count = count;
}

std::size_t type_rule_resolution::get_operand_index() const noexcept
{
	return m_operand_index;
}

bool type_rule_resolution::is_output_operand() const noexcept
{
	return m_output_operand;
}

numerical_type type_rule_resolution::get_offending_type() const noexcept
{
	return m_offending_type;
}

} // namespace rexlib
