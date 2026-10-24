// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/operation/type_rule/type_rule_interpreter.hpp>

#include <core/operation/type_rule/type_rule_engine.hpp>

#include <boost/container/small_vector.hpp>

#include <algorithm>
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
	const type_rule_descriptor &rule
) noexcept
	: m_rule(&rule)
{
}

void type_rule_interpreter::check_instance(
	const operation_descriptor &descriptor,
	span<const numerical_type> operation_data_types
) const
{
	const auto pivots = m_rule->get_pivots();

	const auto read_count = static_cast<std::size_t>(std::count_if(
		pivots.begin(),
		pivots.end(),
		[] (const type_pivot_descriptor &pivot)
		{
			return pivot.get_source() == type_pivot_source::operation;
		}
	));

	if (operation_data_types.size() != read_count)
	{
		std::ostringstream oss;
		oss << descriptor << ": the operation supplies "
			<< operation_data_types.size() << " data types, but its typing "
			<< "rule reads " << read_count << ".";
		throw std::invalid_argument(oss.str());
	}

	std::size_t supplied = 0;
	for (const auto &pivot : pivots)
	{
		if (pivot.get_source() != type_pivot_source::operation)
		{
			continue;
		}

		const auto type = operation_data_types[supplied++];
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
	span<const numerical_type> operation_data_types,
	span<operand_specification> canonical_outputs,
	span<const operand_specification> inputs
) const
{
	auto resolution =
		check_operand_counts(canonical_outputs.size(), inputs.size());
	if (!resolution)
	{
		throw_error(descriptor, resolution);
	}

	const auto input_types = gather_data_types(inputs);
	data_type_buffer output_types(
		canonical_outputs.size(),
		numerical_type::unknown
	);

	resolution = resolve_pivots(
		m_rule->get_pivots(),
		view(input_types),
		operation_data_types
	);
	if (resolution)
	{
		resolution = check_slots(
			m_rule->get_input_slots(),
			resolution,
			view(input_types),
			false
		);
	}
	if (resolution)
	{
		resolution = evaluate_slots(
			m_rule->get_output_slots(),
			resolution,
			make_span(output_types.data(), output_types.size()),
			true
		);
	}
	if (!resolution)
	{
		throw_error(descriptor, resolution);
	}

	for (std::size_t i = 0; i < canonical_outputs.size(); ++i)
	{
		canonical_outputs[i].set_data_type(output_types[i]);
	}
}

void type_rule_interpreter::accept(
	const operation_descriptor &descriptor,
	span<const numerical_type> operation_data_types,
	span<const operand_specification> user_outputs,
	span<const operand_specification> inputs
) const
{
	const auto output_types = gather_data_types(user_outputs);
	const auto input_types = gather_data_types(inputs);

	const auto resolution = resolve(
		operation_data_types,
		view(output_types),
		view(input_types)
	);
	if (!resolution)
	{
		throw_error(descriptor, resolution);
	}
}

type_rule_resolution type_rule_interpreter::resolve(
	span<const numerical_type> operation_data_types,
	span<const numerical_type> output_types,
	span<const numerical_type> input_types
) const noexcept
{
	auto resolution =
		check_operand_counts(output_types.size(), input_types.size());
	if (!resolution)
	{
		return resolution;
	}

	const auto pivots = m_rule->get_pivots();

	resolution = resolve_pivots(pivots, input_types, operation_data_types);
	if (resolution)
	{
		resolution = bind_free_pivots(pivots, resolution, output_types);
	}
	if (resolution)
	{
		resolution = check_slots(
			m_rule->get_input_slots(),
			resolution,
			input_types,
			false
		);
	}
	if (resolution)
	{
		resolution = check_slots(
			m_rule->get_output_slots(),
			resolution,
			output_types,
			true
		);
	}
	return resolution;
}

void type_rule_interpreter::throw_error(
	const operation_descriptor &descriptor,
	const type_rule_resolution &resolution
) const
{
	throw_type_rule_error(
		descriptor,
		resolution,
		resolution.is_output_operand()
			? m_rule->get_output_slots()
			: m_rule->get_input_slots(),
		m_rule->get_pivots()
	);
}

type_rule_resolution type_rule_interpreter::check_operand_counts(
	std::size_t output_count,
	std::size_t input_count
) const noexcept
{
	const auto output_slot_count = m_rule->get_output_slots().size();
	if (output_count != output_slot_count)
	{
		return type_rule_resolution::failure(
			type_rule_status::operand_count_mismatch,
			std::min(output_count, output_slot_count),
			true,
			numerical_type::unknown
		);
	}

	const auto input_slot_count = m_rule->get_input_slots().size();
	if (input_count != input_slot_count)
	{
		return type_rule_resolution::failure(
			type_rule_status::operand_count_mismatch,
			std::min(input_count, input_slot_count),
			false,
			numerical_type::unknown
		);
	}

	return type_rule_resolution();
}

} // namespace rexlib
