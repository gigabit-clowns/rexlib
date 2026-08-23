// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/dispatch/rules/operand_type_rule_engine.hpp>

#include <xmipp4/core/platform/assert.hpp>

#include <sstream>
#include <stdexcept>

namespace xmipp4
{

namespace
{

/**
 * @brief Validate a candidate pivot value against its declaration.
 *
 * @param pivot The pivot descriptor.
 * @param type The candidate value.
 * @param status Set to the reason of the failure. Left untouched on
 * success.
 * @return true The value is admissible.
 * @return false The value is not admissible.
 */
bool check_pivot_value(
	const pivot_descriptor &pivot,
	numerical_type type,
	type_rule_status &status
) noexcept
{
	if (!is_domain_representable(type))
	{
		status = type_rule_status::invalid_type;
		return false;
	}

	if (!pivot.get_domain().contains(type))
	{
		status = type_rule_status::domain_violation;
		return false;
	}

	return true;
}

/**
 * @brief Find the domain constraining an operand, if any.
 *
 * Pivot counts are tiny, so a linear scan is cheaper than carrying the
 * pivot index through the resolution.
 *
 * @param pivots The rule's pivot table.
 * @param operand_index Index of the operand.
 * @param output_operand Whether the operand is an output.
 * @return const numerical_type_domain* The domain, or null when no pivot
 * is sourced from that operand.
 */
const numerical_type_domain* find_operand_domain(
	span<const pivot_descriptor> pivots,
	std::size_t operand_index,
	bool output_operand
) noexcept
{
	const auto source =
		output_operand ? pivot_source::output : pivot_source::input;

	for (const auto &pivot : pivots)
	{
		if (pivot.get_source() == source &&
		    pivot.get_index() == operand_index)
		{
			return &pivot.get_domain();
		}
	}

	return nullptr;
}

} // anonymous namespace

type_rule_resolution resolve_pivots(
	span<const pivot_descriptor> pivots,
	span<const numerical_type> input_types,
	span<const numerical_type> operation_pivots
) noexcept
{
	type_rule_resolution result;
	result.set_pivot_count(pivots.size());

	// First pass, for the pivots whose value is fixed by an operand or by
	// the operation itself.
	for (std::size_t i = 0; i < pivots.size(); ++i)
	{
		const auto &pivot = pivots[i];
		auto status = type_rule_status::ok;

		switch (pivot.get_source())
		{
		case pivot_source::input:
		{
			REX_ASSERT(pivot.get_index() < input_types.size());
			const auto type = input_types[pivot.get_index()];
			if (!check_pivot_value(pivot, type, status))
			{
				return result.fail(
					status,
					pivot.get_index(),
					false,
					type
				);
			}
			result.set_pivot(i, type);
			break;
		}

		case pivot_source::operation:
		{
			if (pivot.get_index() >= operation_pivots.size())
			{
				return result.fail(
					type_rule_status::unresolved_pivot,
					pivot.get_index(),
					false,
					numerical_type::unknown
				);
			}

			const auto type = operation_pivots[pivot.get_index()];
			if (!check_pivot_value(pivot, type, status))
			{
				return result.fail(
					status,
					pivot.get_index(),
					true,
					type
				);
			}
			result.set_pivot(i, type);
			break;
		}

		case pivot_source::output:
			// Free. Either it adopts another pivot below, or it stays
			// undetermined for bind_free_pivots to resolve.
			break;
		}
	}

	// Second pass, for the free pivots that adopt the value of another one
	// when the user did not pre-allocate the output.
	for (std::size_t i = 0; i < pivots.size(); ++i)
	{
		const auto &pivot = pivots[i];
		if (pivot.get_source() != pivot_source::output ||
		    pivot.get_default_pivot() < 0)
		{
			continue;
		}

		const auto source_index =
			static_cast<std::size_t>(pivot.get_default_pivot());
		REX_ASSERT(source_index < pivots.size());
		REX_ASSERT(pivots[source_index].get_source() != pivot_source::output);

		const auto type = result.get_pivot(source_index);
		auto status = type_rule_status::ok;
		if (!check_pivot_value(pivot, type, status))
		{
			return result.fail(
				status,
				pivot.get_index(),
				true,
				type
			);
		}
		result.set_pivot(i, type);
	}

	return result;
}

type_rule_resolution bind_free_pivots(
	span<const pivot_descriptor> pivots,
	const type_rule_resolution &partial,
	span<const numerical_type> user_output_types
) noexcept
{
	auto result = partial;
	if (!result)
	{
		return result;
	}

	for (std::size_t i = 0; i < pivots.size(); ++i)
	{
		const auto &pivot = pivots[i];
		if (pivot.get_source() != pivot_source::output)
		{
			continue;
		}

		REX_ASSERT(pivot.get_index() < user_output_types.size());
		const auto type = user_output_types[pivot.get_index()];
		auto status = type_rule_status::ok;
		if (!check_pivot_value(pivot, type, status))
		{
			return result.fail(
				status,
				pivot.get_index(),
				true,
				type
			);
		}

		// The user's choice overrides any default this pivot adopted, which
		// is what lets a converting operation write a type the inputs did
		// not imply.
		result.set_pivot(i, type);
	}

	return result;
}

type_rule_resolution evaluate_slots(
	span<const slot_descriptor> slots,
	const type_rule_resolution &resolution,
	span<numerical_type> types,
	bool output_operands
) noexcept
{
	REX_ASSERT(slots.size() == types.size());
	REX_ASSERT(resolution);

	for (std::size_t i = 0; i < slots.size(); ++i)
	{
		const auto &slot = slots[i];
		const auto pivot = resolution.get_pivot(slot.get_pivot_index());

		if (!is_domain_representable(pivot))
		{
			// The pivot is still free, so the operand type is not
			// determined yet. Leave it undetermined rather than failing:
			// the user may still supply it as a pre-allocated output.
			types[i] = numerical_type::unknown;
			continue;
		}

		if (!slot.get_transform_domain().contains(pivot))
		{
			return resolution.fail(
				type_rule_status::undefined_transform,
				i,
				output_operands,
				pivot
			);
		}

		types[i] = slot.apply(pivot);
	}

	return resolution;
}

type_rule_resolution check_slots(
	span<const slot_descriptor> slots,
	const type_rule_resolution &resolution,
	span<const numerical_type> types,
	bool output_operands
) noexcept
{
	REX_ASSERT(slots.size() == types.size());
	REX_ASSERT(resolution);

	for (std::size_t i = 0; i < slots.size(); ++i)
	{
		const auto &slot = slots[i];
		const auto pivot = resolution.get_pivot(slot.get_pivot_index());

		if (!is_domain_representable(pivot))
		{
			return resolution.fail(
				type_rule_status::unresolved_pivot,
				i,
				output_operands,
				numerical_type::unknown
			);
		}

		if (!slot.get_transform_domain().contains(pivot))
		{
			return resolution.fail(
				type_rule_status::undefined_transform,
				i,
				output_operands,
				pivot
			);
		}

		if (types[i] != slot.apply(pivot))
		{
			return resolution.fail(
				type_rule_status::slot_mismatch,
				i,
				output_operands,
				types[i]
			);
		}
	}

	return resolution;
}

void throw_type_rule_error(
	const operation_descriptor &descriptor,
	const type_rule_resolution &resolution,
	span<const slot_descriptor> slots,
	span<const pivot_descriptor> pivots
)
{
	REX_ASSERT(!resolution);

	const auto index = resolution.get_operand_index();
	const auto output = resolution.is_output_operand();
	const auto *kind = output ? "output" : "input";
	const auto operand = describe_operand(descriptor, index, output);

	std::ostringstream oss;
	oss << descriptor << ": ";

	switch (resolution.get_status())
	{
	case type_rule_status::invalid_type:
		oss << kind << " operand " << operand << " has data type "
			<< resolution.get_offending_type()
			<< ", which is not a concrete numerical type.";
		break;

	case type_rule_status::domain_violation:
	{
		oss << kind << " operand " << operand << " has data type "
			<< resolution.get_offending_type()
			<< ", which this operation does not accept.";

		const auto *domain = find_operand_domain(pivots, index, output);
		if (domain != nullptr)
		{
			oss << " Expected one of: " << *domain << ".";
		}
		break;
	}

	case type_rule_status::undefined_transform:
	{
		oss << "the data type of " << kind << " operand " << operand
			<< " cannot be derived from element type "
			<< resolution.get_offending_type() << ".";

		if (index < slots.size())
		{
			oss << " It must be " << slots[index].get_relation()
				<< " one of: " << slots[index].get_transform_domain() << ".";
		}
		break;
	}

	case type_rule_status::slot_mismatch:
	{
		oss << kind << " operand " << operand << " has data type "
			<< resolution.get_offending_type();

		// Reporting what was expected needs the pivots the resolution
		// carried into the failure. Building a diagnostic must never be
		// able to fail, so fall back to the bare fact if they are absent.
		const bool has_expected =
			index < slots.size() &&
			slots[index].get_pivot_index() < resolution.get_pivot_count();

		if (has_expected)
		{
			const auto &slot = slots[index];
			oss << ", but this operation requires "
				<< slot.apply(resolution.get_pivot(slot.get_pivot_index()));
		}

		oss << ".";
		break;
	}

	case type_rule_status::unresolved_pivot:
		oss << "the data type of " << kind << " operand " << operand
			<< " is not determined by the inputs. Supply a pre-allocated "
			<< "output to fix it.";
		break;

	case type_rule_status::ok:
		oss << "no error.";
		break;
	}

	throw std::invalid_argument(oss.str());
}

} // namespace xmipp4
