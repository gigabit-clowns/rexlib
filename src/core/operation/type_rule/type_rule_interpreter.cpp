// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/operation/type_rule/type_rule_interpreter.hpp>

#include <core/operation/type_rule/type_rule_engine.hpp>

#include <boost/container/small_vector.hpp>

#include <cstddef>
#include <sstream>
#include <stdexcept>

namespace rexlib
{

namespace
{

using data_type_buffer = boost::container::small_vector<numerical_type, 4>;

data_type_buffer gather_data_types(
	span<const operand_specification> operands
)
{
	data_type_buffer result;
	result.reserve(operands.size());
	for (const auto &operand : operands)
	{
		result.push_back(operand.get_data_type());
	}
	return result;
}

span<const numerical_type> view(const data_type_buffer &types) noexcept
{
	return make_span(types.data(), types.size());
}

} // anonymous namespace

type_rule_interpreter::type_rule_interpreter(
	span<const type_pivot_descriptor> pivots,
	span<const type_slot_descriptor> output_slots,
	span<const type_slot_descriptor> input_slots
) noexcept
	: m_pivots(pivots)
	, m_output_slots(output_slots)
	, m_input_slots(input_slots)
{
}

void type_rule_interpreter::check_instance(
	const operation_descriptor &descriptor,
	span<const numerical_type> operation_pivots
) const
{
	std::size_t read_count = 0;
	for (const auto &pivot : m_pivots)
	{
		if (pivot.get_source() == type_pivot_source::operation)
		{
			++read_count;
		}
	}

	if (operation_pivots.size() != read_count)
	{
		std::ostringstream oss;
		oss << descriptor << ": the operation supplies "
			<< operation_pivots.size() << " data types, but its typing rule "
			<< "reads " << read_count << ".";
		throw std::invalid_argument(oss.str());
	}

	std::size_t supplied = 0;
	for (const auto &pivot : m_pivots)
	{
		if (pivot.get_source() != type_pivot_source::operation)
		{
			continue;
		}

		const auto type = operation_pivots[supplied++];
		if (!is_domain_representable(type) ||
			!pivot.get_domain().contains(type))
		{
			std::ostringstream oss;
			oss << descriptor << ": the operation fixes data type " << type
				<< ", but this operation requires one of: "
				<< pivot.get_domain() << ".";
			throw std::invalid_argument(oss.str());
		}
	}
}

void type_rule_interpreter::deduce(
	const operation_descriptor &descriptor,
	span<const numerical_type> operation_pivots,
	span<operand_specification> canonical_outputs,
	span<const operand_specification> inputs
) const
{
	check_operand_counts(descriptor, canonical_outputs.size(), inputs.size());

	const auto input_types = gather_data_types(inputs);
	data_type_buffer output_types(
		canonical_outputs.size(),
		numerical_type::unknown
	);

	auto resolution = resolve_pivots(
		m_pivots,
		view(input_types),
		operation_pivots
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
	resolution = check_slots(
		m_input_slots,
		resolution,
		view(input_types),
		false
	);
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
		make_span(output_types.data(), output_types.size()),
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

	for (std::size_t i = 0; i < canonical_outputs.size(); ++i)
	{
		canonical_outputs[i].set_data_type(output_types[i]);
	}
}

void type_rule_interpreter::accept(
	const operation_descriptor &descriptor,
	span<const numerical_type> operation_pivots,
	span<const operand_specification> user_outputs,
	span<const operand_specification> inputs
) const
{
	check_operand_counts(descriptor, user_outputs.size(), inputs.size());

	const auto input_types = gather_data_types(inputs);
	const auto user_output_types = gather_data_types(user_outputs);

	auto resolution = resolve_pivots(
		m_pivots,
		view(input_types),
		operation_pivots
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

	resolution = bind_free_pivots(
		m_pivots,
		resolution,
		view(user_output_types)
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

	resolution = check_slots(
		m_input_slots,
		resolution,
		view(input_types),
		false
	);
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
		view(user_output_types),
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

span<const type_slot_descriptor>
type_rule_interpreter::get_slots_for_report(
	bool output_operands
) const noexcept
{
	return output_operands ? m_output_slots : m_input_slots;
}

void type_rule_interpreter::check_operand_counts(
	const operation_descriptor &descriptor,
	std::size_t output_count,
	std::size_t input_count
) const
{
	if (output_count != m_output_slots.size() ||
		input_count != m_input_slots.size())
	{
		std::ostringstream oss;
		oss << descriptor << ": the typing rule describes "
			<< m_output_slots.size() << " outputs and "
			<< m_input_slots.size() << " inputs, but " << output_count
			<< " and " << input_count << " were given.";
		throw std::invalid_argument(oss.str());
	}
}

} // namespace rexlib
