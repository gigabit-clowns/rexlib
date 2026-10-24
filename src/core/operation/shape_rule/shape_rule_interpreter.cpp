// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/operation/shape_rule/shape_rule_interpreter.hpp>

#include "shape_rule_resolution.hpp"

#include <rexlib/core/layout/broadcast.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <algorithm>
#include <iterator>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rexlib
{

namespace
{

using shape_type = operand_specification::shape_type;
using axis_list = std::vector<std::size_t>;

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
	const auto extents = rule.get_extents();
	const auto read_count = static_cast<std::size_t>(
		std::count_if(
			extents.begin(),
			extents.end(),
			[] (const extent_pivot_descriptor &extent)
			{
				return extent.get_source() == extent_pivot_source::operation;
			}
		)
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
	for (const auto &extent : extents)
	{
		if (extent.get_source() != extent_pivot_source::operation)
		{
			continue;
		}

		const auto value = operation_extents[supplied++];
		if (!extent.get_domain().contains(value))
		{
			std::ostringstream oss;
			oss << descriptor << ": the operation fixes an extent of "
				<< value << ", but this operation requires "
				<< extent.get_domain().get_description() << ".";
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

	axis_list sorted(named_axes.begin(), named_axes.end());
	std::sort(sorted.begin(), sorted.end());
	const auto repeated = std::adjacent_find(sorted.begin(), sorted.end());
	if (repeated != sorted.end())
	{
		std::ostringstream oss;
		oss << descriptor << ": axis " << *repeated
			<< " is named more than once.";
		throw std::invalid_argument(oss.str());
	}

	if (rule.get_input_cores().empty())
	{
		std::ostringstream oss;
		oss << descriptor << ": the shape rule names axes, but describes no "
			<< "input to name them in.";
		throw std::invalid_argument(oss.str());
	}

	const auto &core = rule.get_input_cores()[0];
	const auto required = core.get_axis_count();
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

std::size_t compute_frame_rank(span<const operand_specification> inputs)
{
	std::size_t result = 0;
	for (const auto &input : inputs)
	{
		result = std::max(result, input.get_shape().size());
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
 * @return axis_list The axis of the input holding every core axis, in the
 * order the core axes are declared.
 */
axis_list locate_input_core(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	std::size_t input,
	const shape_type &shape,
	span<const std::size_t> named_axes,
	std::size_t frame_rank
)
{
	const auto rank = shape.size();
	axis_list result;

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
	return core.has_run() ? core_axis_count - core.get_axis_count() : 0;
}

/**
 * @brief Find the first core axis a slot stands for.
 *
 * Every slot stands for one core axis, except a run, which stands for
 * @p run_length of them.
 */
std::size_t locate_slot(
	const core_dimensions_descriptor &core,
	std::size_t slot,
	std::size_t run_length
) noexcept
{
	return slot <= core.get_run_position() ? slot : slot - 1 + run_length;
}

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
	const std::vector<axis_list> &input_cores
)
{
	const auto extents = rule.get_extents();
	shape_type values(extents.size());
	std::vector<bool> bound(extents.size(), false);

	std::size_t supplied = 0;
	for (std::size_t e = 0; e < extents.size(); ++e)
	{
		if (extents[e].get_source() == extent_pivot_source::operation)
		{
			values[e] = operation_extents[supplied++];
			bound[e] = true;
		}
	}

	for (std::size_t i = 0; i < inputs.size(); ++i)
	{
		const auto &core = rule.get_input_cores()[i];
		const auto &shape = inputs[i].get_shape();
		const auto slots = core.get_slots();
		const auto run_length =
			compute_run_length(core, input_cores[i].size());

		for (std::size_t j = 0; j < slots.size(); ++j)
		{
			const auto &slot = slots[j];
			if (slot.get_kind() != axis_slot_kind::extent ||
				!slot.determines_extent() ||
				bound[slot.get_index()])
			{
				continue;
			}

			const auto axis =
				input_cores[i][locate_slot(core, j, run_length)];
			const auto extent = shape[axis];
			const auto &domain = extents[slot.get_index()].get_domain();
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

			values[slot.get_index()] = extent;
			bound[slot.get_index()] = true;
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
 * @brief Write where the value of an extent variable comes from.
 */
void write_extent_origin(
	std::ostream &os,
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	const shape_type &values,
	const std::vector<axis_list> &input_cores,
	std::size_t extent
)
{
	const auto source = rule.get_extents()[extent].get_source();
	if (source == extent_pivot_source::operation)
	{
		os << values[extent] << ", which the operation fixes";
		return;
	}

	for (std::size_t i = 0; i < input_cores.size(); ++i)
	{
		const auto &core = rule.get_input_cores()[i];
		const auto slots = core.get_slots();
		const auto run_length =
			compute_run_length(core, input_cores[i].size());

		for (std::size_t j = 0; j < slots.size(); ++j)
		{
			const auto &slot = slots[j];
			if (slot.get_kind() == axis_slot_kind::extent &&
				slot.determines_extent() &&
				slot.get_index() == extent)
			{
				os << "axis "
					<< input_cores[i][locate_slot(core, j, run_length)]
					<< " of input operand "
					<< describe_operand(descriptor, i, false);
				return;
			}
		}
	}
}

void throw_core_mismatch(
	const operation_descriptor &descriptor,
	std::size_t input,
	std::size_t axis,
	std::size_t extent,
	std::size_t expected,
	const std::string &reason
)
{
	std::ostringstream oss;
	oss << descriptor << ": input operand "
		<< describe_operand(descriptor, input, false) << " has extent "
		<< extent << " along axis " << axis
		<< ", but this operation requires " << expected << reason << ".";
	throw std::invalid_argument(oss.str());
}

/**
 * @brief Check every core axis of the inputs against its slot.
 *
 * @return shape_type The extents of the axes of the run, as the first
 * input carrying it has them.
 */
shape_type check_input_cores(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	const shape_type &values,
	span<const operand_specification> inputs,
	const std::vector<axis_list> &input_cores
)
{
	shape_type run_extents;
	std::size_t run_origin = no_index;
	axis_list run_origin_axes;

	for (std::size_t i = 0; i < inputs.size(); ++i)
	{
		const auto &core = rule.get_input_cores()[i];
		const auto &shape = inputs[i].get_shape();
		const auto slots = core.get_slots();
		const auto run_length =
			compute_run_length(core, input_cores[i].size());

		for (std::size_t j = 0; j < slots.size(); ++j)
		{
			const auto &slot = slots[j];
			const auto first = locate_slot(core, j, run_length);

			switch (slot.get_kind())
			{
			case axis_slot_kind::extent:
			{
				const auto axis = input_cores[i][first];
				const auto expected = slot.apply(values[slot.get_index()]);
				if (shape[axis] != expected)
				{
					std::ostringstream reason;
					reason << ", " << slot.get_relation() << ' ';
					write_extent_origin(
						reason,
						descriptor,
						rule,
						values,
						input_cores,
						slot.get_index()
					);
					throw_core_mismatch(
						descriptor,
						i,
						axis,
						shape[axis],
						expected,
						reason.str()
					);
				}
				break;
			}

			case axis_slot_kind::fixed:
			{
				const auto axis = input_cores[i][first];
				if (shape[axis] != slot.get_index())
				{
					throw_core_mismatch(
						descriptor,
						i,
						axis,
						shape[axis],
						slot.get_index(),
						""
					);
				}
				break;
			}

			case axis_slot_kind::run:
			{
				const auto binds = run_origin == no_index;
				for (std::size_t k = 0; k < run_length; ++k)
				{
					const auto axis = input_cores[i][first + k];
					const auto extent = shape[axis];
					if (binds)
					{
						if (!slot.get_domain().contains(extent))
						{
							throw_extent_outside_domain(
								descriptor,
								i,
								axis,
								extent,
								slot.get_domain()
							);
						}
						run_extents.push_back(extent);
						run_origin_axes.push_back(axis);
					}
					else if (extent != run_extents[k])
					{
						std::ostringstream reason;
						reason << ", the extent of axis " << run_origin_axes[k]
							<< " of input operand "
							<< describe_operand(descriptor, run_origin, false);
						throw_core_mismatch(
							descriptor,
							i,
							axis,
							extent,
							run_extents[k],
							reason.str()
						);
					}
				}
				if (binds)
				{
					run_origin = i;
				}
				break;
			}
			}
		}
	}

	return run_extents;
}

/**
 * @brief Find the axis of an input lying at a position of the batch frame.
 *
 * @return std::size_t The axis, or no_index when the input does not reach
 * that far.
 */
std::size_t find_batch_axis(
	std::size_t rank,
	std::size_t offset,
	std::size_t position
) noexcept
{
	return position >= offset && position - offset < rank
		? position - offset
		: no_index;
}

/**
 * @brief Broadcast the batch axes of every input together.
 *
 * The batch axes are aligned in a frame: the last axes before the core ones
 * under trailing placement, and the axes the inputs broadcast to under named
 * placement, the named ones left out.
 */
shape_type broadcast_batches(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	span<const operand_specification> inputs,
	const std::vector<axis_list> &input_cores,
	span<const std::size_t> named_axes,
	std::size_t frame_rank
)
{
	const auto trailing = rule.get_placement() == core_placement::trailing;

	// The rank of every input aligned in the frame, its core axes left out
	// under trailing placement.
	std::vector<std::size_t> aligned_ranks;
	aligned_ranks.reserve(inputs.size());
	for (std::size_t i = 0; i < inputs.size(); ++i)
	{
		const auto rank = inputs[i].get_shape().size();
		aligned_ranks.push_back(trailing ? rank - input_cores[i].size() : rank);
	}

	const auto frame = trailing
		? (aligned_ranks.empty()
			? 0
			: *std::max_element(aligned_ranks.begin(), aligned_ranks.end()))
		: frame_rank;

	std::vector<bool> named(frame, false);
	if (!trailing)
	{
		for (const auto axis : named_axes)
		{
			named[axis] = true;
		}
	}

	shape_type extents(frame, 1);
	for (std::size_t i = 0; i < inputs.size(); ++i)
	{
		const auto &shape = inputs[i].get_shape();
		const auto offset = frame - aligned_ranks[i];

		for (std::size_t q = 0; q < aligned_ranks[i]; ++q)
		{
			const auto position = q + offset;
			const auto extent = shape[q];
			if (named[position] || extent == 1)
			{
				continue;
			}

			if (extents[position] == 1)
			{
				extents[position] = extent;
				continue;
			}

			if (extents[position] == extent)
			{
				continue;
			}

			// Reported against the first input holding the other extent.
			for (std::size_t other = 0; other < i; ++other)
			{
				const auto other_axis = find_batch_axis(
					aligned_ranks[other],
					frame - aligned_ranks[other],
					position
				);
				if (other_axis == no_index ||
					inputs[other].get_shape()[other_axis] != extents[position])
				{
					continue;
				}

				std::ostringstream oss;
				oss << descriptor << ": input operands "
					<< describe_operand(descriptor, other, false) << " and "
					<< describe_operand(descriptor, i, false)
					<< " do not broadcast together: axis " << other_axis
					<< " of the first has extent " << extents[position]
					<< " and axis " << q << " of the second has extent "
					<< extent << ".";
				throw std::invalid_argument(oss.str());
			}
		}
	}

	shape_type result;
	result.reserve(frame);
	for (std::size_t position = 0; position < frame; ++position)
	{
		if (!named[position])
		{
			result.push_back(extents[position]);
		}
	}
	return result;
}

shape_rule_resolution resolve(
	const operation_descriptor &descriptor,
	const shape_rule_descriptor &rule,
	span<const std::size_t> named_axes,
	span<const std::size_t> operation_extents,
	std::size_t output_count,
	span<const operand_specification> inputs
)
{
	check_operand_counts(descriptor, rule, output_count, inputs.size());
	check_operation_extents(descriptor, rule, operation_extents);
	check_named_axes(descriptor, rule, named_axes);

	const auto frame_rank = compute_frame_rank(inputs);
	check_named_axes_in_frame(descriptor, named_axes, frame_rank);

	std::vector<axis_list> input_cores;
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
	auto batch = broadcast_batches(
		descriptor,
		rule,
		inputs,
		input_cores,
		named_axes,
		frame_rank
	);

	return shape_rule_resolution(
		std::move(batch),
		std::move(values),
		std::move(run_extents)
	);
}

shape_type make_output_shape(
	const shape_rule_descriptor &rule,
	const shape_rule_resolution &resolution,
	span<const std::size_t> named_axes,
	bool keep_dimensions,
	std::size_t output
)
{
	shape_type core_extents;
	for (const auto &slot : rule.get_output_cores()[output].get_slots())
	{
		switch (slot.get_kind())
		{
		case axis_slot_kind::extent:
			core_extents.push_back(
				slot.apply(resolution.get_extent(slot.get_index()))
			);
			break;

		case axis_slot_kind::fixed:
			core_extents.push_back(slot.get_index());
			break;

		case axis_slot_kind::run:
			core_extents.insert(
				core_extents.end(),
				resolution.get_run_extents().begin(),
				resolution.get_run_extents().end()
			);
			break;
		}
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

shape_rule_interpreter::shape_rule_interpreter(
	const shape_rule_descriptor &rule
) noexcept
	: m_rule(&rule)
{
}

void shape_rule_interpreter::check_instance(
	const operation_descriptor &descriptor,
	span<const std::size_t> named_axes,
	span<const std::size_t> operation_extents
) const
{
	check_operation_extents(descriptor, *m_rule, operation_extents);
	check_named_axes(descriptor, *m_rule, named_axes);
}

void shape_rule_interpreter::deduce(
	const operation_descriptor &descriptor,
	span<const std::size_t> named_axes,
	bool keep_dimensions,
	span<const std::size_t> operation_extents,
	span<operand_specification> canonical_outputs,
	span<const operand_specification> inputs
) const
{
	const auto resolution = resolve(
		descriptor,
		*m_rule,
		named_axes,
		operation_extents,
		canonical_outputs.size(),
		inputs
	);

	for (std::size_t o = 0; o < canonical_outputs.size(); ++o)
	{
		canonical_outputs[o].set_shape(
			make_output_shape(
				*m_rule,
				resolution,
				named_axes,
				keep_dimensions,
				o
			)
		);
	}
}

void shape_rule_interpreter::accept(
	const operation_descriptor &descriptor,
	span<const std::size_t> named_axes,
	bool keep_dimensions,
	span<const std::size_t> operation_extents,
	span<const operand_specification> user_outputs,
	span<const operand_specification> inputs
) const
{
	const auto resolution = resolve(
		descriptor,
		*m_rule,
		named_axes,
		operation_extents,
		user_outputs.size(),
		inputs
	);

	if (m_rule->get_output_batch() == output_batch::broadcast_target)
	{
		check_broadcast_target_outputs(
			descriptor,
			*m_rule,
			resolution,
			user_outputs
		);
		return;
	}

	check_exact_outputs(
		descriptor,
		*m_rule,
		resolution,
		named_axes,
		keep_dimensions,
		user_outputs
	);
}

} // namespace rexlib
