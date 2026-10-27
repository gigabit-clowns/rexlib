// SPDX-License-Identifier: GPL-3.0-only

#include "shape_rule_engine.hpp"

#include <rexlib/core/layout/broadcast.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/core/platform/attributes.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <algorithm>
#include <iterator>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace rexlib
{

namespace
{

using shape_type = operand_specification::shape_type;
using axis_list_type = std::vector<std::size_t>;

REXLIB_CONST_CONSTEXPR std::size_t no_index =
	std::numeric_limits<std::size_t>::max();

void write_count(
	std::ostream &os,
	std::size_t count,
	const char *singular,
	const char *plural
)
{
	os << count << ' ' << (count == 1 ? singular : plural);
}

void write_shape(std::ostream &os, const shape_type &shape)
{
	os << '(';
	for (std::size_t i = 0; i < shape.size(); ++i)
	{
		if (i > 0)
		{
			os << ", ";
		}
		os << shape[i];
	}
	os << ')';
}

bool is_supplied(const extent_pivot_descriptor &pivot) noexcept
{
	return pivot.get_source() != extent_pivot_source::inputs;
}

bool carries_named_axes(
	const shape_rule_descriptor &rule,
	std::size_t input
) noexcept
{
	return
		rule.get_placement() == core_placement::named &&
		!rule.get_input_cores()[input].is_empty();
}

void check_operand_counts(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	std::size_t output_count,
	std::size_t input_count
)
{
	const auto rule_output_count = rule.get_output_cores().size();
	const auto rule_input_count = rule.get_input_cores().size();
	if (output_count != rule_output_count || input_count != rule_input_count)
	{
		std::ostringstream oss;
		oss << descriptor << ": the shape rule describes ";
		write_count(oss, rule_output_count, "output", "outputs");
		oss << " and ";
		write_count(oss, rule_input_count, "input", "inputs");
		oss << ", but ";
		write_count(oss, output_count, "output", "outputs");
		oss << " and ";
		write_count(oss, input_count, "input", "inputs");
		oss << " were given.";
		throw std::invalid_argument(oss.str());
	}
}

void check_operation_extents(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	span<const std::size_t> operation_extents
)
{
	const auto pivots = rule.get_pivots();
	const auto read_count = static_cast<std::size_t>(
		std::count_if(pivots.begin(), pivots.end(), &is_supplied)
	);

	if (operation_extents.size() != read_count)
	{
		std::ostringstream oss;
		oss << descriptor << ": the operation supplies ";
		write_count(oss, operation_extents.size(), "extent", "extents");
		oss << ", but its shape rule reads " << read_count << ".";
		throw std::invalid_argument(oss.str());
	}

	std::size_t supplied = 0;
	for (const auto &pivot : pivots)
	{
		if (!is_supplied(pivot))
		{
			continue;
		}

		const auto value = operation_extents[supplied++];
		if (!pivot.get_domain().contains(value))
		{
			std::ostringstream oss;
			oss << descriptor << ": the operation fixes an extent of "
				<< value << ", but this operation requires "
				<< pivot.get_domain().get_description() << ".";
			throw std::invalid_argument(oss.str());
		}
	}
}

void check_named_axes(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	span<const std::size_t> named_axes
)
{
	if (rule.get_placement() == core_placement::trailing)
	{
		if (!named_axes.empty())
		{
			std::ostringstream oss;
			oss << descriptor << ": ";
			write_count(oss, named_axes.size(), "axis is", "axes are");
			oss << " named, but this operation acts along the last axes of "
				<< "its operands.";
			throw std::invalid_argument(oss.str());
		}
		return;
	}

	axis_list_type sorted(named_axes.begin(), named_axes.end());
	std::sort(sorted.begin(), sorted.end());
	const auto repeated = std::adjacent_find(sorted.begin(), sorted.end());
	if (repeated != sorted.end())
	{
		std::ostringstream oss;
		oss << descriptor << ": axis " << *repeated
			<< " is named more than once.";
		throw std::invalid_argument(oss.str());
	}

	const auto cores = rule.get_input_cores();
	const auto carrier = std::find_if(
		cores.begin(),
		cores.end(),
		[](const core_dimensions_descriptor &core) { return !core.is_empty(); }
	);
	if (carrier == cores.end())
	{
		std::ostringstream oss;
		oss << descriptor << ": the shape rule names axes, but describes no "
			<< "input to name them in.";
		throw std::invalid_argument(oss.str());
	}

	const auto &core = *carrier;
	const auto required = core.get_slots().size();
	const auto admissible = core.has_run()
		? named_axes.size() >= required
		: named_axes.size() == required;
	if (!admissible)
	{
		std::ostringstream oss;
		oss << descriptor << ": ";
		write_count(oss, named_axes.size(), "axis is", "axes are");
		oss << " named, but this operation acts along "
			<< (core.has_run() ? "at least " : "exactly ");
		write_count(oss, required, "axis", "axes");
		oss << ".";
		throw std::invalid_argument(oss.str());
	}
}

/**
 * @brief Get the rank of the frame the named axes are named in.
 *
 * Under named placement only the inputs carrying the named axes take part.
 */
std::size_t compute_frame_rank(
	const shape_rule_descriptor &rule,
	span<const operand_specification> inputs
)
{
	const auto named = rule.get_placement() == core_placement::named;

	std::size_t result = 0;
	for (std::size_t i = 0; i < inputs.size(); ++i)
	{
		if (!named || carries_named_axes(rule, i))
		{
			result = std::max(result, inputs[i].get_shape().size());
		}
	}
	return result;
}

void check_named_axes_in_frame(
	const operation_descriptor &descriptor,
	span<const std::size_t> named_axes,
	std::size_t frame_rank
)
{
	for (const auto axis : named_axes)
	{
		if (axis >= frame_rank)
		{
			std::ostringstream oss;
			oss << descriptor << ": axis " << axis
				<< " was named, but the inputs only have ";
			write_count(oss, frame_rank, "axis", "axes");
			oss << ".";
			throw std::out_of_range(oss.str());
		}
	}
}

/**
 * @brief Find the core axes of an input.
 *
 * @return axis_list_type The axis of the input holding every core axis, the run
 * first.
 */
axis_list_type locate_input_core(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	std::size_t input,
	const shape_type &shape,
	span<const std::size_t> named_axes,
	std::size_t frame_rank
)
{
	const auto rank = shape.size();
	axis_list_type result;

	if (rule.get_placement() == core_placement::trailing)
	{
		const auto count = rule.get_input_cores()[input].get_slots().size();
		if (rank < count)
		{
			std::ostringstream oss;
			oss << descriptor << ": input operand "
				<< describe_operand(descriptor, input, false) << " has ";
			write_count(oss, rank, "axis", "axes");
			oss << ", but this operation needs at least " << count
				<< " of them.";
			throw std::invalid_argument(oss.str());
		}

		for (auto axis = rank - count; axis < rank; ++axis)
		{
			result.push_back(axis);
		}
		return result;
	}

	if (!carries_named_axes(rule, input))
	{
		const auto batch_rank = frame_rank - named_axes.size();
		if (rank > batch_rank)
		{
			std::ostringstream oss;
			oss << descriptor << ": input operand "
				<< describe_operand(descriptor, input, false) << " has ";
			write_count(oss, rank, "axis", "axes");
			oss << ", but the inputs carrying the named axes have only ";
			write_count(oss, batch_rank, "other axis", "other axes");
			oss << " for it to broadcast against.";
			throw std::invalid_argument(oss.str());
		}
		return result;
	}

	const auto offset = frame_rank - rank;
	for (const auto axis : named_axes)
	{
		if (axis < offset)
		{
			std::ostringstream oss;
			oss << descriptor << ": input operand "
				<< describe_operand(descriptor, input, false)
				<< " has no axis " << axis << ": its ";
			write_count(oss, rank, "axis is", "axes are");
			oss << " the last of the " << frame_rank
				<< " the inputs broadcast to.";
			throw std::invalid_argument(oss.str());
		}
		result.push_back(axis - offset);
	}
	return result;
}

std::size_t compute_run_length(
	const core_dimensions_descriptor &core,
	std::size_t core_axis_count
) noexcept
{
	return core.has_run() ? core_axis_count - core.get_slots().size() : 0;
}

REXLIB_NORETURN
void throw_extent_outside_domain(
	const operation_descriptor &descriptor,
	std::size_t input,
	std::size_t axis,
	std::size_t extent,
	const extent_domain &domain
)
{
	std::ostringstream oss;
	oss << descriptor << ": input operand "
		<< describe_operand(descriptor, input, false) << " has extent "
		<< extent << " along axis " << axis
		<< ", but this operation requires " << domain.get_description()
		<< ".";
	throw std::invalid_argument(oss.str());
}

shape_type bind_extents(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	span<const std::size_t> operation_extents,
	span<const operand_specification> inputs,
	const std::vector<axis_list_type> &input_cores
)
{
	const auto pivots = rule.get_pivots();
	shape_type values(pivots.size());
	std::vector<bool> bound(pivots.size(), false);

	std::size_t supplied = 0;
	for (std::size_t p = 0; p < pivots.size(); ++p)
	{
		if (is_supplied(pivots[p]))
		{
			values[p] = operation_extents[supplied++];
			bound[p] = true;
		}
	}

	for (std::size_t i = 0; i < inputs.size(); ++i)
	{
		const auto core = rule.get_input_cores()[i];
		const auto &shape = inputs[i].get_shape();
		const auto slots = core.get_slots();
		const auto run_length =
			compute_run_length(core, input_cores[i].size());

		for (std::size_t j = 0; j < slots.size(); ++j)
		{
			const auto pivot = slots[j].get_pivot_index();
			if (!slots[j].get_transform().is_identity() || bound[pivot])
			{
				continue;
			}

			const auto axis = input_cores[i][run_length + j];
			const auto extent = shape[axis];
			const auto &domain = pivots[pivot].get_domain();
			if (!domain.contains(extent))
			{
				throw_extent_outside_domain(
					descriptor,
					i,
					axis,
					extent,
					domain
				);
			}

			values[pivot] = extent;
			bound[pivot] = true;
		}
	}

	const auto unbound = std::find(bound.begin(), bound.end(), false);
	if (unbound != bound.end())
	{
		std::ostringstream oss;
		oss << descriptor << ": the shape rule leaves extent variable "
			<< std::distance(bound.begin(), unbound)
			<< " undetermined, since no input axis carries it as it is.";
		throw std::invalid_argument(oss.str());
	}

	return values;
}

/**
 * @brief Write the input axis an extent variable was bound from.
 */
void write_binding_axis(
	std::ostream &os,
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	const std::vector<axis_list_type> &input_cores,
	std::size_t pivot
)
{
	for (std::size_t i = 0; i < input_cores.size(); ++i)
	{
		const auto core = rule.get_input_cores()[i];
		const auto slots = core.get_slots();
		const auto run_length =
			compute_run_length(core, input_cores[i].size());

		for (std::size_t j = 0; j < slots.size(); ++j)
		{
			if (slots[j].get_pivot_index() == pivot &&
				slots[j].get_transform().is_identity())
			{
				os << "axis " << input_cores[i][run_length + j]
					<< " of input operand "
					<< describe_operand(descriptor, i, false);
				return;
			}
		}
	}
}

/**
 * @brief Write where the extent a slot requires comes from.
 *
 * An extent read through the identity from a value the operation supplies
 * or fixes is written without the relation.
 */
void write_expected_origin(
	std::ostream &os,
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	const shape_type &values,
	const std::vector<axis_list_type> &input_cores,
	const extent_slot_descriptor &slot
)
{
	const auto pivot = slot.get_pivot_index();
	const auto &transform = slot.get_transform();
	const auto source = rule.get_pivots()[pivot].get_source();

	if (source == extent_pivot_source::inputs)
	{
		os << ", " << transform.get_relation() << ' ';
		write_binding_axis(os, descriptor, rule, input_cores, pivot);
		return;
	}

	if (!transform.is_identity())
	{
		os << ", " << transform.get_relation() << ' ' << values[pivot];
	}

	if (source == extent_pivot_source::operation)
	{
		os << ", which the operation fixes";
	}
}

REXLIB_NORETURN
void throw_core_mismatch(
	const operation_descriptor &descriptor,
	std::size_t input,
	std::size_t axis,
	std::size_t extent,
	std::size_t expected,
	const std::string &origin
)
{
	std::ostringstream oss;
	oss << descriptor << ": input operand "
		<< describe_operand(descriptor, input, false) << " has extent "
		<< extent << " along axis " << axis
		<< ", but this operation requires " << expected << origin << ".";
	throw std::invalid_argument(oss.str());
}

/**
 * @brief Take the extents of the run from the first input carrying it.
 */
void bind_run(
	const operation_descriptor &descriptor,
	std::size_t input,
	const shape_type &shape,
	span<const std::size_t> run_axes,
	const extent_domain &domain,
	shape_type &run_extents,
	axis_list_type &run_origin_axes
)
{
	for (const auto axis : run_axes)
	{
		if (!domain.contains(shape[axis]))
		{
			throw_extent_outside_domain(
				descriptor,
				input,
				axis,
				shape[axis],
				domain
			);
		}
		run_extents.push_back(shape[axis]);
		run_origin_axes.push_back(axis);
	}
}

/**
 * @brief Check the run of an input against the run of the first input
 * carrying one.
 */
void check_run(
	const operation_descriptor &descriptor,
	std::size_t input,
	const shape_type &shape,
	span<const std::size_t> run_axes,
	const shape_type &run_extents,
	const axis_list_type &run_origin_axes,
	std::size_t run_origin
)
{
	for (std::size_t k = 0; k < run_axes.size(); ++k)
	{
		const auto extent = shape[run_axes[k]];
		if (extent == run_extents[k])
		{
			continue;
		}

		std::ostringstream origin;
		origin << ", the extent of axis " << run_origin_axes[k]
			<< " of input operand "
			<< describe_operand(descriptor, run_origin, false);
		throw_core_mismatch(
			descriptor,
			input,
			run_axes[k],
			extent,
			run_extents[k],
			origin.str()
		);
	}
}

/**
 * @brief Check the core axes of an input after the run against their
 * slots.
 */
void check_input_slots(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	const shape_type &values,
	const std::vector<axis_list_type> &input_cores,
	std::size_t input,
	const shape_type &shape
)
{
	const auto core = rule.get_input_cores()[input];
	const auto run_length =
		compute_run_length(core, input_cores[input].size());
	const auto slots = core.get_slots();

	for (std::size_t j = 0; j < slots.size(); ++j)
	{
		const auto &slot = slots[j];
		const auto axis = input_cores[input][run_length + j];
		const auto expected =
			slot.get_transform().apply(values[slot.get_pivot_index()]);
		if (shape[axis] == expected)
		{
			continue;
		}

		std::ostringstream origin;
		write_expected_origin(
			origin,
			descriptor,
			rule,
			values,
			input_cores,
			slot
		);
		throw_core_mismatch(
			descriptor,
			input,
			axis,
			shape[axis],
			expected,
			origin.str()
		);
	}
}

/**
 * @brief Check every core axis of the inputs: the run, then the slots.
 *
 * @return shape_type The extents of the axes of the run, as the first
 * input carrying it has them.
 */
shape_type check_input_cores(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	const shape_type &values,
	span<const operand_specification> inputs,
	const std::vector<axis_list_type> &input_cores
)
{
	shape_type run_extents;
	std::size_t run_origin = no_index;
	axis_list_type run_origin_axes;

	for (std::size_t i = 0; i < inputs.size(); ++i)
	{
		const auto core = rule.get_input_cores()[i];
		const auto &shape = inputs[i].get_shape();
		const auto run_axes = make_span(
			input_cores[i].data(),
			compute_run_length(core, input_cores[i].size())
		);

		if (core.has_run() && run_origin == no_index)
		{
			bind_run(
				descriptor,
				i,
				shape,
				run_axes,
				core.get_run_domain(),
				run_extents,
				run_origin_axes
			);
			run_origin = i;
		}
		else if (core.has_run())
		{
			check_run(
				descriptor,
				i,
				shape,
				run_axes,
				run_extents,
				run_origin_axes,
				run_origin
			);
		}

		check_input_slots(descriptor, rule, values, input_cores, i, shape);
	}

	return run_extents;
}

/**
 * @brief List the batch axes of an input: those not among its core axes,
 * ascending.
 */
axis_list_type list_batch_axes(
	std::size_t rank,
	const axis_list_type &core_axes
)
{
	std::vector<bool> core(rank, false);
	for (const auto axis : core_axes)
	{
		core[axis] = true;
	}

	axis_list_type result;
	result.reserve(rank - core_axes.size());
	for (std::size_t axis = 0; axis < rank; ++axis)
	{
		if (!core[axis])
		{
			result.push_back(axis);
		}
	}
	return result;
}

/**
 * @brief Find the batch axis of an input lying at a position of the batch
 * frame, the batch axes being aligned to its end.
 *
 * @return std::size_t The axis, or no_index when the input does not reach
 * that far.
 */
std::size_t find_batch_axis(
	const axis_list_type &batch_axes,
	std::size_t frame,
	std::size_t position
) noexcept
{
	const auto offset = frame - batch_axes.size();
	return position >= offset ? batch_axes[position - offset] : no_index;
}

/**
 * @brief Find the first input before another whose batch axis at a
 * position of the frame has an extent.
 *
 * @return std::size_t The input, or no_index when none has.
 */
std::size_t find_batch_origin(
	span<const operand_specification> inputs,
	const std::vector<axis_list_type> &batch_axes,
	std::size_t frame,
	std::size_t position,
	std::size_t extent,
	std::size_t before
) noexcept
{
	for (std::size_t other = 0; other < before; ++other)
	{
		const auto axis = find_batch_axis(batch_axes[other], frame, position);
		if (axis != no_index && inputs[other].get_shape()[axis] == extent)
		{
			return other;
		}
	}
	return no_index;
}

/**
 * @brief Report two inputs whose batch axes do not broadcast together.
 */
REXLIB_NORETURN
void throw_batch_mismatch(
	const operation_descriptor &descriptor,
	span<const operand_specification> inputs,
	std::size_t first,
	std::size_t first_axis,
	std::size_t second,
	std::size_t second_axis
)
{
	std::ostringstream oss;
	oss << descriptor << ": input operands "
		<< describe_operand(descriptor, first, false) << " and "
		<< describe_operand(descriptor, second, false)
		<< " do not broadcast together: axis " << first_axis
		<< " of the first has extent "
		<< inputs[first].get_shape()[first_axis]
		<< " and axis " << second_axis << " of the second has extent "
		<< inputs[second].get_shape()[second_axis] << ".";
	throw std::invalid_argument(oss.str());
}

/**
 * @brief Broadcast the batch axes of every input together.
 *
 * The batch axes of an input are its axes less its core axes. They are
 * aligned to the end of the batch frame, whichever the placement: under
 * named placement the batch axes of an input carrying the named axes are
 * already the last of the axes of the frame that are not named.
 */
shape_type broadcast_batches(
	const operation_descriptor &descriptor,
	span<const operand_specification> inputs,
	const std::vector<axis_list_type> &input_cores
)
{
	std::vector<axis_list_type> batch_axes;
	batch_axes.reserve(inputs.size());
	std::size_t frame = 0;
	for (std::size_t i = 0; i < inputs.size(); ++i)
	{
		batch_axes.push_back(
			list_batch_axes(inputs[i].get_shape().size(), input_cores[i])
		);
		frame = std::max(frame, batch_axes.back().size());
	}

	shape_type extents(frame, 1);
	for (std::size_t i = 0; i < inputs.size(); ++i)
	{
		const auto &shape = inputs[i].get_shape();
		const auto offset = frame - batch_axes[i].size();

		for (std::size_t k = 0; k < batch_axes[i].size(); ++k)
		{
			const auto position = k + offset;
			const auto axis = batch_axes[i][k];
			const auto extent = shape[axis];
			if (extent == 1 || extent == extents[position])
			{
				continue;
			}

			if (extents[position] != 1)
			{
				const auto origin = find_batch_origin(
					inputs,
					batch_axes,
					frame,
					position,
					extents[position],
					i
				);
				REXLIB_ASSERT(origin != no_index);
				throw_batch_mismatch(
					descriptor,
					inputs,
					origin,
					find_batch_axis(batch_axes[origin], frame, position),
					i,
					axis
				);
			}
			extents[position] = extent;
		}
	}

	return extents;
}

void check_exact_outputs(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	const shape_rule_resolution &resolution,
	span<const std::size_t> named_axes,
	bool keep_dimensions,
	span<const operand_specification> user_outputs
)
{
	for (std::size_t o = 0; o < user_outputs.size(); ++o)
	{
		const auto expected = make_output_shape(
			rule,
			resolution,
			named_axes,
			keep_dimensions,
			o
		);
		const auto &shape = user_outputs[o].get_shape();
		if (shape != expected)
		{
			std::ostringstream oss;
			oss << descriptor << ": output operand "
				<< describe_operand(descriptor, o, true) << " has shape ";
			write_shape(oss, shape);
			oss << ", but the inputs call for ";
			write_shape(oss, expected);
			oss << ".";
			throw std::invalid_argument(oss.str());
		}
	}
}

void check_broadcast_target_outputs(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	const shape_rule_resolution &resolution,
	span<const operand_specification> user_outputs
)
{
	if (user_outputs.empty())
	{
		return;
	}

	const auto &target = user_outputs[0].get_shape();
	for (std::size_t o = 1; o < user_outputs.size(); ++o)
	{
		if (user_outputs[o].get_shape() != target)
		{
			std::ostringstream oss;
			oss << descriptor << ": output operand "
				<< describe_operand(descriptor, o, true)
				<< " does not have the shape of output operand "
				<< describe_operand(descriptor, 0, true)
				<< ", which an operation without core axes requires.";
			throw std::invalid_argument(oss.str());
		}
	}

	const auto deduced = make_output_shape(rule, resolution, {}, false, 0);
	if (!is_broadcastable_to(make_span(deduced), make_span(target)))
	{
		std::ostringstream oss;
		oss << descriptor << ": output operand "
			<< describe_operand(descriptor, 0, true) << " has shape ";
		write_shape(oss, target);
		oss << ", which the inputs, broadcast to ";
		write_shape(oss, deduced);
		oss << ", cannot be written into.";
		throw std::invalid_argument(oss.str());
	}
}

} // anonymous namespace

void check_shape_rule_instance(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	span<const std::size_t> named_axes,
	span<const std::size_t> operation_extents
)
{
	check_operation_extents(descriptor, rule, operation_extents);
	check_named_axes(descriptor, rule, named_axes);
}

shape_rule_resolution resolve_shape_rule(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	span<const std::size_t> named_axes,
	span<const std::size_t> operation_extents,
	std::size_t output_count,
	span<const operand_specification> inputs
)
{
	check_operand_counts(descriptor, rule, output_count, inputs.size());
	check_shape_rule_instance(descriptor, rule, named_axes, operation_extents);

	const auto frame_rank = compute_frame_rank(rule, inputs);
	check_named_axes_in_frame(descriptor, named_axes, frame_rank);

	std::vector<axis_list_type> input_cores;
	input_cores.reserve(inputs.size());
	for (std::size_t i = 0; i < inputs.size(); ++i)
	{
		input_cores.push_back(
			locate_input_core(
				descriptor,
				rule,
				i,
				inputs[i].get_shape(),
				named_axes,
				frame_rank
			)
		);
	}

	auto values = bind_extents(
		descriptor,
		rule,
		operation_extents,
		inputs,
		input_cores
	);
	auto run_extents = check_input_cores(
		descriptor,
		rule,
		values,
		inputs,
		input_cores
	);
	auto batch = broadcast_batches(descriptor, inputs, input_cores);

	return shape_rule_resolution(
		std::move(batch),
		std::move(values),
		std::move(run_extents)
	);
}

operand_specification::shape_type make_output_shape(
	const shape_rule_descriptor &rule,
	const shape_rule_resolution &resolution,
	span<const std::size_t> named_axes,
	bool keep_dimensions,
	std::size_t output
)
{
	const auto core = rule.get_output_cores()[output];

	shape_type core_extents;
	if (core.has_run())
	{
		core_extents = resolution.get_run_extents();
	}
	for (const auto &slot : core.get_slots())
	{
		core_extents.push_back(
			slot.get_transform().apply(
				resolution.get_extent(slot.get_pivot_index())
			)
		);
	}

	const auto &batch = resolution.get_batch();
	if (rule.get_placement() == core_placement::trailing)
	{
		shape_type result = batch;
		result.insert(result.end(), core_extents.begin(), core_extents.end());
		return result;
	}

	const auto frame = batch.size() + named_axes.size();
	std::vector<std::size_t> named_position(frame, no_index);
	for (std::size_t k = 0; k < named_axes.size(); ++k)
	{
		named_position[named_axes[k]] = k;
	}

	shape_type result;
	result.reserve(frame);
	auto next_batch = batch.begin();
	for (std::size_t position = 0; position < frame; ++position)
	{
		const auto k = named_position[position];
		if (k == no_index)
		{
			result.push_back(*next_batch++);
		}
		else if (!core_extents.empty())
		{
			result.push_back(core_extents[k]);
		}
		else if (keep_dimensions)
		{
			result.push_back(1);
		}
	}
	return result;
}

void check_output_shapes(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	const shape_rule_resolution &resolution,
	span<const std::size_t> named_axes,
	bool keep_dimensions,
	span<const operand_specification> user_outputs
)
{
	if (rule.get_output_batch() == output_batch::broadcast_target)
	{
		check_broadcast_target_outputs(
			descriptor,
			rule,
			resolution,
			user_outputs
		);
		return;
	}

	check_exact_outputs(
		descriptor,
		rule,
		resolution,
		named_axes,
		keep_dimensions,
		user_outputs
	);
}

} // namespace rexlib
