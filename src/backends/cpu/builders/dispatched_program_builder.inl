// SPDX-License-Identifier: GPL-3.0-only

#include "dispatched_program_builder.hpp"

#include <rex/core/dispatch/operation.hpp>
#include <rex/core/dispatch/operation_cast.hpp>
#include <rex/core/platform/constexpr.hpp>

#include <backends/cpu/builders/dispatcher_support_query.hpp>
#include <backends/cpu/hardware/functor_program.hpp>

#include <stdexcept>
#include <string>
#include <utility>

namespace rexlib
{
namespace cpu
{

template <typename Derived, typename Op, typename TypeDispatcher>
const Derived&
dispatched_program_builder<Derived, Op, TypeDispatcher>
::get_derived() const noexcept
{
	return static_cast<const Derived&>(*this);
}

template <typename Derived, typename Op, typename TypeDispatcher>
bool
dispatched_program_builder<Derived, Op, TypeDispatcher>::accepts_signatures(
	span<const operand_signature> /*output_signatures*/,
	span<const operand_signature> /*input_signatures*/
) const noexcept
{
	return true;
}

template <typename Derived, typename Op, typename TypeDispatcher>
operation_id
dispatched_program_builder<Derived, Op, TypeDispatcher>
::get_operation_id() const noexcept
{
	return operation_id::of<Op>();
}

template <typename Derived, typename Op, typename TypeDispatcher>
backend_priority
dispatched_program_builder<Derived, Op, TypeDispatcher>::get_suitability(
	const operation &operation,
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures,
	rexlib::command_queue &queue
) const
{
	REXLIB_CONST_CONSTEXPR auto output_count =
		Op::output_operand_count;
	REXLIB_CONST_CONSTEXPR auto input_count =
		Op::input_operand_count;

	const auto base = program_builder::get_suitability(
		operation,
		output_signatures,
		input_signatures,
		queue
	);
	if (base == backend_priority::unsupported)
	{
		return base;
	}

	if (output_signatures.size() != output_count ||
	    input_signatures.size() != input_count)
	{
		return backend_priority::unsupported;
	}

	if (!get_derived().accepts_signatures(output_signatures, input_signatures))
	{
		return backend_priority::unsupported;
	}

	std::array<numerical_type, output_count> output_types;
	std::array<numerical_type, input_count> input_types;
	extract_data_types(output_types, output_signatures);
	extract_data_types(input_types, input_signatures);

	// Reporting an unsupported element type here, rather than throwing from
	// build(), is what lets the manager fall through to another backend.
	const auto supported =
		detail::dispatcher_support_query<TypeDispatcher>::is_supported(
			make_span(output_types.data(), output_count),
			make_span(input_types.data(), input_count)
		);
	if (!supported)
	{
		return backend_priority::unsupported;
	}

	return base;
}

template <typename Derived, typename Op, typename TypeDispatcher>
std::shared_ptr<rexlib::program>
dispatched_program_builder<Derived, Op, TypeDispatcher>::build(
	const operation &operation,
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures,
	rexlib::command_queue& /*queue*/,
	program_cache* /*cache*/
) const
{
	REXLIB_CONST_CONSTEXPR auto output_count =
		Op::output_operand_count;
	REXLIB_CONST_CONSTEXPR auto input_count =
		Op::input_operand_count;

	const auto &typed_operation = operation_cast<Op>(operation);

	// The operation names itself, so the message says which one was asked
	// for rather than which family was asked.
	if (output_signatures.size() != output_count)
	{
		throw std::invalid_argument(
			std::string(Op::get_static_descriptor().get_name()) +
			": Unexpected output signature count."
		);
	}
	if (input_signatures.size() != input_count)
	{
		throw std::invalid_argument(
			std::string(Op::get_static_descriptor().get_name()) +
			": Unexpected input signature count."
		);
	}

	std::array<numerical_type, output_count> output_types;
	std::array<numerical_type, input_count> input_types;
	extract_data_types(output_types, output_signatures);
	extract_data_types(input_types, input_signatures);

	// The whole partition of the iteration space is settled here, once per
	// program rather than once per element.
	const Derived &derived = get_derived();
	auto plan = derived.make_plan(
		typed_operation,
		output_signatures,
		input_signatures
	);

	return TypeDispatcher::dispatch(
		Op::get_static_descriptor(),
		[&derived, &typed_operation, &plan]
		(auto output_element_types, auto input_element_types)
		{
			return make_functor_program(
				derived.make_loop_functor(
					typed_operation,
					plan,
					output_element_types,
					input_element_types
				),
				output_element_types,
				input_element_types
			);
		},
		output_types,
		input_types
	);
}

} // namespace cpu
} // namespace rexlib
