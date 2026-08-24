// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/dispatch/rules/rule_operation_data_type_policy.hpp>

#include <rex/core/dispatch/rules/operand_type_rule_engine.hpp>
#include <rex/core/platform/assert.hpp>

#include <algorithm>

namespace rex
{

rule_operation_data_type_policy::rule_operation_data_type_policy(
	span<const pivot_descriptor> pivots,
	span<const slot_descriptor> output_slots,
	span<const slot_descriptor> input_slots
) noexcept
	: m_pivots(pivots)
	, m_output_slots(output_slots)
	, m_input_slots(input_slots)
	, m_operation_pivot_count(0)
{
	std::fill(
		m_operation_pivots.begin(),
		m_operation_pivots.end(),
		numerical_type::unknown
	);
}

rule_operation_data_type_policy::rule_operation_data_type_policy(
	span<const pivot_descriptor> pivots,
	span<const slot_descriptor> output_slots,
	span<const slot_descriptor> input_slots,
	std::initializer_list<numerical_type> operation_pivots
) noexcept
	: rule_operation_data_type_policy(pivots, output_slots, input_slots)
{
	REXLIB_ASSERT(operation_pivots.size() <= max_operand_type_pivot_count);

	m_operation_pivot_count = std::min(
		operation_pivots.size(),
		max_operand_type_pivot_count
	);
	std::copy_n(
		operation_pivots.begin(),
		m_operation_pivot_count,
		m_operation_pivots.begin()
	);
}

rule_operation_data_type_policy::~rule_operation_data_type_policy() = default;

span<const numerical_type>
rule_operation_data_type_policy::get_operation_pivots() const noexcept
{
	return make_span(m_operation_pivots.data(), m_operation_pivot_count);
}

span<const slot_descriptor>
rule_operation_data_type_policy::get_slots_for_report(
	bool output_operands
) const noexcept
{
	return output_operands ? m_output_slots : m_input_slots;
}

void rule_operation_data_type_policy::deduce(
	const operation_descriptor &descriptor,
	span<numerical_type> canonical_output_types,
	span<const numerical_type> input_types
) const
{
	auto resolution = resolve_pivots(
		m_pivots,
		input_types,
		get_operation_pivots()
	);
	if (!resolution)
	{
		throw_type_rule_error(
			descriptor,
			resolution,
			get_slots_for_report(resolution.is_output_operand()),
			m_pivots
		);
	}

	// Enforce the relation the rule declares between the inputs. For a
	// homogeneous rule this is what rejects mismatching operands.
	resolution = check_slots(m_input_slots, resolution, input_types, false);
	if (!resolution)
	{
		throw_type_rule_error(
			descriptor,
			resolution,
			m_input_slots,
			m_pivots
		);
	}

	resolution = evaluate_slots(
		m_output_slots,
		resolution,
		canonical_output_types,
		true
	);
	if (!resolution)
	{
		throw_type_rule_error(
			descriptor,
			resolution,
			m_output_slots,
			m_pivots
		);
	}
}

void rule_operation_data_type_policy::accept(
	const operation_descriptor &descriptor,
	span<const numerical_type> user_output_types,
	span<const numerical_type> /*canonical_output_types*/,
	span<const numerical_type> input_types
) const
{
	// The canonical types are not consulted: re-resolving the rule against
	// the user supplied outputs is both equivalent and more precise, since
	// it is what binds the pivots the inputs left free.
	auto resolution = resolve_pivots(
		m_pivots,
		input_types,
		get_operation_pivots()
	);
	if (!resolution)
	{
		throw_type_rule_error(
			descriptor,
			resolution,
			get_slots_for_report(resolution.is_output_operand()),
			m_pivots
		);
	}

	resolution = bind_free_pivots(m_pivots, resolution, user_output_types);
	if (!resolution)
	{
		throw_type_rule_error(
			descriptor,
			resolution,
			m_output_slots,
			m_pivots
		);
	}

	resolution = check_slots(m_input_slots, resolution, input_types, false);
	if (!resolution)
	{
		throw_type_rule_error(
			descriptor,
			resolution,
			m_input_slots,
			m_pivots
		);
	}

	resolution = check_slots(
		m_output_slots,
		resolution,
		user_output_types,
		true
	);
	if (!resolution)
	{
		throw_type_rule_error(
			descriptor,
			resolution,
			m_output_slots,
			m_pivots
		);
	}
}

} // namespace rex
