// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/functional/reconstruction.hpp>

#include <rexlib/core/dispatch/execute.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/ops/reconstruction/insert_slices_operation.hpp>
#include <rexlib/em/ops/reconstruction/insert_weighted_slices_operation.hpp>

#include <array>
#include <cstddef>
#include <utility>

namespace rexlib
{
namespace em
{

namespace
{

template <std::size_t InputCount>
std::pair<array, array> execute_insertion(
	const operation &op,
	const std::array<const_array_ref, InputCount> &inputs,
	const execution_context &context,
	array *numerator_out,
	array *denominator_out
)
{
	std::array<array, 2> outputs;
	if (numerator_out)
	{
		outputs[0] = numerator_out->share();
	}
	if (denominator_out)
	{
		outputs[1] = denominator_out->share();
	}

	execute(op, make_span(outputs), make_span(inputs), context);

	return std::make_pair(std::move(outputs[0]), std::move(outputs[1]));
}

} // anonymous namespace

std::pair<array, array> insert_slices(
	const_array_ref numerator,
	const_array_ref denominator,
	const_array_ref slices,
	const_array_ref transforms,
	const_array_ref radial_weights,
	const gridding_window &window,
	const execution_context &context,
	array *numerator_out,
	array *denominator_out
)
{
	const std::array<const_array_ref, 5> inputs = {
		numerator,
		denominator,
		slices,
		transforms,
		radial_weights
	};
	return execute_insertion(
		insert_slices_operation(window),
		inputs,
		context,
		numerator_out,
		denominator_out
	);
}

std::pair<array, array> insert_weighted_slices(
	const_array_ref numerator,
	const_array_ref denominator,
	const_array_ref slices,
	const_array_ref weights,
	const_array_ref transforms,
	const_array_ref radial_weights,
	const gridding_window &window,
	const execution_context &context,
	array *numerator_out,
	array *denominator_out
)
{
	const std::array<const_array_ref, 6> inputs = {
		numerator,
		denominator,
		slices,
		weights,
		transforms,
		radial_weights
	};
	return execute_insertion(
		insert_weighted_slices_operation(window),
		inputs,
		context,
		numerator_out,
		denominator_out
	);
}

} // namespace em
} // namespace rexlib
