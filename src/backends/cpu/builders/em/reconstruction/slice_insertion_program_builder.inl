// SPDX-License-Identifier: GPL-3.0-only

#include "slice_insertion_program_builder.hpp"

#include <rexlib/core/operation/shape_rule/shape_rule_descriptor_of.hpp>

#include <array>
#include <cstddef>
#include <utility>

namespace rexlib
{
namespace cpu
{

template <typename Op, typename SampleWeight>
inline
slice_insertion_layout_plan
slice_insertion_program_builder<Op, SampleWeight>::make_plan(
	const Op&,
	span<const operand_descriptor> output_descriptors,
	span<const operand_descriptor> input_descriptors
) const
{
	using shape_rule_type = typename Op::shape_rule_type;
	const auto cores =
		shape_rule_descriptor_of<shape_rule_type>::get().get_input_cores();

	std::array<std::size_t, Op::input_operand_count> core_ranks{};
	for (std::size_t i = 0; i < core_ranks.size(); ++i)
	{
		core_ranks[i] = cores[i].get_slots().size();
	}

	return slice_insertion_layout_plan(
		output_descriptors,
		input_descriptors,
		make_span(core_ranks)
	);
}

template <typename Op, typename SampleWeight>
template <typename... Outs, typename... Ins>
inline
slice_insertion_functor<
	typename type_list_element<1, type_list<Outs...>>::type,
	SampleWeight
>
slice_insertion_program_builder<Op, SampleWeight>::make_loop_functor(
	const Op &operation,
	slice_insertion_layout_plan &plan,
	type_list<Outs...>,
	type_list<Ins...>
) const
{
	using real_type = typename type_list_element<1, type_list<Outs...>>::type;
	return slice_insertion_functor<real_type, SampleWeight>(
		std::move(plan),
		operation.get_window(),
		SampleWeight::make_parameters(operation)
	);
}

} // namespace cpu
} // namespace rexlib
