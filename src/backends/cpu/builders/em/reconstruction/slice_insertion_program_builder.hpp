// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "slice_insertion_functor.hpp"

#include <backends/cpu/builders/dispatched_program_builder.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>
#include <backends/cpu/plans/slice_insertion_layout_plan.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/span.hpp>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Builds the program of a slice insertion.
 *
 * @tparam Op The operation. Its shape rule gives the core of each input.
 * @tparam SampleWeight The weights of a sample in the accumulators.
 */
template <typename Op, typename SampleWeight>
class slice_insertion_program_builder final
	: public dispatched_program_builder<
		slice_insertion_program_builder<Op, SampleWeight>,
		Op,
		rule_type_dispatcher<typename Op::type_rule_type>
	>
{
public:
	slice_insertion_program_builder() noexcept = default;

	slice_insertion_layout_plan make_plan(
		const Op &operation,
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors
	) const;

	template <typename... Outs, typename... Ins>
	slice_insertion_functor<
		typename type_list_element<1, type_list<Outs...>>::type,
		SampleWeight
	>
	make_loop_functor(
		const Op &operation,
		slice_insertion_layout_plan &plan,
		type_list<Outs...> output_element_types,
		type_list<Ins...> input_element_types
	) const;
};

} // namespace cpu
} // namespace rexlib

#include "slice_insertion_program_builder.inl"
