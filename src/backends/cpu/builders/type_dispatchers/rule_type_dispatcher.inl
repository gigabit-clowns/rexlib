// SPDX-License-Identifier: GPL-3.0-only

#include "rule_type_dispatcher.hpp"

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/numerical/numerical_type_dispatch.hpp>
#include <rexlib/core/operation/type_rule/type_rule_descriptor_of.hpp>

#include <stdexcept>

namespace rexlib
{
namespace cpu
{

template <typename Rule, template <typename...> class Support>
type_rule_interpreter
rule_type_dispatcher<Rule, Support>::get_interpreter() noexcept
{
	return type_rule_interpreter(type_rule_descriptor_of<Rule>::get());
}

template <typename Rule, template <typename...> class Support>
template <std::size_t... PivotIndices>
bool rule_type_dispatcher<Rule, Support>::check_support(
	const type_rule_resolution &resolution,
	std::index_sequence<PivotIndices...>
)
{
	// Only the predicate is instantiated here, never the factory, so
	// asking is cheap even for a rule with several pivots.
	return dispatch_numerical_types(
		[] (auto... tags) -> bool
		{
			return is_admissible<typename decltype(tags)::type...>::value;
		},
		resolution.get_pivot(PivotIndices)...
	);
}

template <typename Rule, template <typename...> class Support>
bool rule_type_dispatcher<Rule, Support>::is_supported(
	span<const numerical_type> output_types,
	span<const numerical_type> input_types,
	span<const numerical_type> operation_data_types
) noexcept
{
	const auto resolution = get_interpreter().resolve(
		operation_data_types,
		output_types,
		input_types
	);
	if (!resolution)
	{
		return false;
	}

	try
	{
		return check_support(resolution, pivot_indices());
	}
	catch (...)
	{
		// Being asked whether a configuration is supported is never an
		// error, so anything unexpected answers no rather than escaping.
		return false;
	}
}

template <typename Rule, template <typename...> class Support>
template <typename F, typename Pivots>
std::shared_ptr<rexlib::program>
rule_type_dispatcher<Rule, Support>::invoke_if(
	std::true_type /*supported*/,
	F &&factory,
	Pivots /*pivot_types*/
)
{
	return std::forward<F>(factory)(
		type_rule_operand_types_t<
			typename Rule::output_slot_list_type,
			Pivots
		>(),
		type_rule_operand_types_t<
			typename Rule::input_slot_list_type,
			Pivots
		>()
	);
}

template <typename Rule, template <typename...> class Support>
template <typename F, typename Pivots>
std::shared_ptr<rexlib::program>
rule_type_dispatcher<Rule, Support>::invoke_if(
	std::false_type /*supported*/,
	F&& /*factory*/,
	Pivots /*pivot_types*/
)
{
	throw std::invalid_argument(
		"rule_type_dispatcher: this backend does not support the requested "
		"element type."
	);
}

template <typename Rule, template <typename...> class Support>
template <typename F, std::size_t... PivotIndices>
std::shared_ptr<rexlib::program>
rule_type_dispatcher<Rule, Support>::dispatch_pivots(
	F &&factory,
	const type_rule_resolution &resolution,
	std::index_sequence<PivotIndices...>
)
{
	return dispatch_numerical_types(
		[&factory] (auto... tags) -> std::shared_ptr<rexlib::program>
		{
			using pivot_types = type_list<typename decltype(tags)::type...>;

			// The operation's own domain gates instantiation, so a kernel
			// never has to compile for an element type the operation
			// does not accept.
			using admissible = typename is_admissible<
				typename decltype(tags)::type...
			>::type;

			return invoke_if(
				admissible(),
				std::forward<F>(factory),
				pivot_types()
			);
		},
		resolution.get_pivot(PivotIndices)...
	);
}

template <typename Rule, template <typename...> class Support>
template <typename F, std::size_t OutputCount, std::size_t InputCount>
std::shared_ptr<rexlib::program>
rule_type_dispatcher<Rule, Support>::dispatch(
	const operation_descriptor &descriptor,
	F &&factory,
	const std::array<numerical_type, OutputCount> &output_types,
	const std::array<numerical_type, InputCount> &input_types,
	span<const numerical_type> operation_data_types
)
{
	static_assert(
		OutputCount == Rule::output_count,
		"The operand arrays do not match the output arity of the rule."
	);
	static_assert(
		InputCount == Rule::input_count,
		"The operand arrays do not match the input arity of the rule."
	);

	const auto interpreter = get_interpreter();
	const auto resolution = interpreter.resolve(
		operation_data_types,
		make_span(output_types.data(), OutputCount),
		make_span(input_types.data(), InputCount)
	);
	if (!resolution)
	{
		interpreter.throw_error(descriptor, resolution);
	}

	return dispatch_pivots(
		std::forward<F>(factory),
		resolution,
		pivot_indices()
	);
}

} // namespace cpu
} // namespace rexlib
