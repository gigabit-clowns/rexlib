// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "core_dimensions.hpp"
#include "core_placement.hpp"
#include "operand_axis_slot.hpp"
#include "operand_extent.hpp"
#include "output_batch.hpp"
#include "same_extent_transform.hpp"
#include "trailing_core.hpp"

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace rexlib
{

namespace detail
{

template <bool... Values>
struct shape_rule_bool_pack;

template <bool... Values>
struct shape_rule_all_of
	: std::is_same<
		shape_rule_bool_pack<true, Values...>,
		shape_rule_bool_pack<Values..., true>
	>
{
};

template <bool... Values>
struct shape_rule_any_of
	: std::integral_constant<bool, !shape_rule_all_of<(!Values)...>::value>
{
};

template <std::size_t ExtentCount, typename Core>
struct core_names_declared_extents;

template <std::size_t ExtentCount, typename... Slots>
struct core_names_declared_extents<ExtentCount, core_dimensions<Slots...>>
	: shape_rule_all_of<(
		Slots::kind != axis_slot_kind::extent ||
		Slots::index < ExtentCount
	)...>
{
};

template <std::size_t ExtentCount, typename Cores>
struct cores_name_declared_extents;

template <std::size_t ExtentCount, typename... Cores>
struct cores_name_declared_extents<ExtentCount, type_list<Cores...>>
	: shape_rule_all_of<
		core_names_declared_extents<ExtentCount, Cores>::value...
	>
{
};

template <std::size_t Extent, typename Core>
struct core_binds_extent;

template <std::size_t Extent, typename... Slots>
struct core_binds_extent<Extent, core_dimensions<Slots...>>
	: shape_rule_any_of<(
		Slots::kind == axis_slot_kind::extent &&
		Slots::index == Extent &&
		std::is_same<
			typename Slots::transform_type,
			same_extent_transform
		>::value
	)...>
{
};

template <std::size_t Extent, typename Cores>
struct cores_bind_extent;

template <std::size_t Extent, typename... Cores>
struct cores_bind_extent<Extent, type_list<Cores...>>
	: shape_rule_any_of<core_binds_extent<Extent, Cores>::value...>
{
};

template <typename Extents, typename InputCores, typename Indices>
struct input_extents_bound;

template <typename... Extents, typename InputCores, std::size_t... Indices>
struct input_extents_bound<
	type_list<Extents...>,
	InputCores,
	std::index_sequence<Indices...>
>
	: shape_rule_all_of<(
		Extents::source != extent_source::inputs ||
		cores_bind_extent<Indices, InputCores>::value
	)...>
{
};

template <typename Extents>
struct extents_from_operation;

template <typename... Extents>
struct extents_from_operation<type_list<Extents...>>
	: shape_rule_any_of<(Extents::source == extent_source::operation)...>
{
};

template <typename Cores>
struct cores_without_run;

template <typename... Cores>
struct cores_without_run<type_list<Cores...>>
	: shape_rule_all_of<(Cores::run_count == 0)...>
{
};

template <typename Cores>
struct cores_empty;

template <typename... Cores>
struct cores_empty<type_list<Cores...>>
	: shape_rule_all_of<(Cores::slot_count == 0)...>
{
};

template <typename Cores>
struct cores_not_empty;

template <typename... Cores>
struct cores_not_empty<type_list<Cores...>>
	: shape_rule_all_of<(Cores::slot_count > 0)...>
{
};

template <std::size_t AxisCount, std::size_t RunCount, typename Cores>
struct cores_empty_or_alike;

template <std::size_t AxisCount, std::size_t RunCount, typename... Cores>
struct cores_empty_or_alike<AxisCount, RunCount, type_list<Cores...>>
	: shape_rule_all_of<(
		Cores::slot_count == 0 ||
		(Cores::axis_count == AxisCount && Cores::run_count == RunCount)
	)...>
{
};

template <typename Inputs, typename Outputs>
struct named_cores_alike
	: std::true_type
{
};

template <typename First, typename... Rest, typename Outputs>
struct named_cores_alike<type_list<First, Rest...>, Outputs>
	: std::integral_constant<
		bool,
		cores_empty_or_alike<
			First::axis_count,
			First::run_count,
			type_list<First, Rest...>
		>::value &&
		cores_empty_or_alike<
			First::axis_count,
			First::run_count,
			Outputs
		>::value
	>
{
};

} // namespace detail

/**
 * @brief Declarative shape rule for the operands of an operation.
 *
 * Every axis of an operand is either a batch axis or a core axis. The
 * batch axes of all operands broadcast together, while the core axes are
 * declared per operand by a core_dimensions, whose slots relate them through
 * a small set of extent variables. Core axes never broadcast: every axis
 * naming an extent variable must agree with it.
 *
 * The rule is lowered to tables, so a single interpreter serves every
 * operation. Values that depend on the operation instance, such as the axes
 * it names, are read through the accessors the rule declares.
 *
 * @tparam Extents type_list of operand_extent declarations.
 * @tparam Outputs type_list of core_dimensions, one per output operand, in
 * signature order.
 * @tparam Inputs type_list of core_dimensions, one per input operand, in
 * signature order.
 * @tparam Placement Where the core axes lie: trailing_core or named_core.
 * @tparam Batch Which batch axes an output supplied in advance may have.
 *
 * @see operand_extent
 * @see core_dimensions
 */
template <
	typename Extents,
	typename Outputs,
	typename Inputs,
	typename Placement = trailing_core,
	output_batch Batch = output_batch::exact
>
struct operand_shape_rule
{
	using extent_list = Extents;
	using output_core_list = Outputs;
	using input_core_list = Inputs;
	using placement_type = Placement;

	static REXLIB_CONST_CONSTEXPR output_batch batch = Batch;

	static REXLIB_CONST_CONSTEXPR std::size_t extent_count =
		type_list_size<Extents>::value;
	static REXLIB_CONST_CONSTEXPR std::size_t output_count =
		type_list_size<Outputs>::value;
	static REXLIB_CONST_CONSTEXPR std::size_t input_count =
		type_list_size<Inputs>::value;

	/**
	 * @brief Whether the rule reads values of the operation instance.
	 */
	static REXLIB_CONST_CONSTEXPR bool requires_parameters =
		Placement::placement == core_placement::named ||
		detail::extents_from_operation<Extents>::value;

	static_assert(
		extent_count <= max_operand_extent_count,
		"A shape rule declares more extent variables than a resolution can "
		"hold. Raise max_operand_extent_count if this is genuinely needed."
	);

	static_assert(
		detail::cores_name_declared_extents<extent_count, Outputs>::value &&
		detail::cores_name_declared_extents<extent_count, Inputs>::value,
		"A core axis names an extent variable the rule does not declare."
	);

	static_assert(
		detail::input_extents_bound<
			Extents,
			Inputs,
			std::make_index_sequence<extent_count>
		>::value,
		"An extent variable bound by the inputs is not carried untransformed "
		"by any input axis, so nothing determines it."
	);

	static_assert(
		Placement::placement == core_placement::named ||
		(
			detail::cores_without_run<Outputs>::value &&
			detail::cores_without_run<Inputs>::value
		),
		"A run is as long as the axes the operation instance names, so only "
		"a rule whose core axes are named may declare one."
	);

	static_assert(
		Placement::placement != core_placement::named || input_count > 0,
		"A rule whose core axes are named needs an input to name them in."
	);

	static_assert(
		Placement::placement != core_placement::named ||
		detail::cores_not_empty<Inputs>::value,
		"Under named placement every input carries the named axes, so no "
		"input may declare empty core dimensions."
	);

	static_assert(
		Placement::placement != core_placement::named ||
		detail::named_cores_alike<Inputs, Outputs>::value,
		"Under named placement every core carrying the named axes must "
		"declare as many axes and runs as the others."
	);

	static_assert(
		Batch != output_batch::broadcast_target ||
		(
			Placement::placement == core_placement::trailing &&
			detail::cores_empty<Outputs>::value &&
			detail::cores_empty<Inputs>::value
		),
		"Only an operation without core axes may write into outputs larger "
		"than its inputs broadcast to."
	);
};

} // namespace rexlib
