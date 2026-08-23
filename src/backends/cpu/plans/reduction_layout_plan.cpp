// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/plans/reduction_layout_plan.hpp>

#include <rex/core/dispatch/operand_signature.hpp>
#include <rex/core/layout/broadcast.hpp>
#include <rex/core/layout/joint_layout_builder.hpp>
#include <rex/core/layout/strided_layout.hpp>

#include <rex/core/platform/constexpr.hpp>

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace rex
{
namespace cpu
{

namespace
{

/**
 * @brief The iteration space the inputs share.
 *
 * Broadcasting the inputs together before reducing is what lets one reduction
 * take an operand alongside the one being folded, such as a mask or a second
 * vector, and is what the shape policy did to deduce the output shape.
 */
std::vector<std::size_t> broadcast_input_extents(
	span<const operand_signature> input_signatures
)
{
	std::vector<std::size_t> extents;
	input_signatures[0].get_layout().get_extents(extents);

	std::vector<std::size_t> other;
	for (std::size_t i = 1; i < input_signatures.size(); ++i)
	{
		input_signatures[i].get_layout().get_extents(other);
		broadcast_extents_accumulate(extents, make_span(other));
	}

	return extents;
}

template <typename T>
std::vector<T> select_axes(
	const std::vector<T> &values,
	const std::vector<std::size_t> &axes
)
{
	std::vector<T> result;
	result.reserve(axes.size());
	for (const auto axis : axes)
	{
		result.push_back(values[axis]);
	}
	return result;
}

std::size_t element_count(const std::vector<std::size_t> &extents) noexcept
{
	std::size_t result = 1;
	for (const auto extent : extents)
	{
		result *= extent;
	}
	return result;
}

/**
 * @brief Strides of an output along the axes that survive the reduction.
 *
 * With the reduced axes kept, the output has the rank of the iteration space
 * and the surviving axes sit where they always were. With them dropped, the
 * output only has the surviving axes, in the same ascending order.
 */
std::vector<std::ptrdiff_t> map_output_strides(
	const strided_layout &layout,
	const std::vector<std::size_t> &kept_axes,
	std::size_t rank,
	bool keep_dimensions
)
{
	const auto expected_rank = keep_dimensions ? rank : kept_axes.size();
	if (layout.get_rank() != expected_rank)
	{
		std::ostringstream oss;
		oss << "reduction_layout_plan: An output of rank "
			<< expected_rank << " was expected, but one of rank "
			<< layout.get_rank() << " was given.";
		throw std::invalid_argument(oss.str());
	}

	std::vector<std::ptrdiff_t> strides;
	layout.get_strides(strides);

	std::vector<std::ptrdiff_t> result;
	result.reserve(kept_axes.size());
	for (std::size_t i = 0; i < kept_axes.size(); ++i)
	{
		result.push_back(strides[keep_dimensions ? kept_axes[i] : i]);
	}

	return result;
}

} // anonymous namespace

reduction_layout_plan::reduction_layout_plan(
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures,
	span<const std::size_t> axes,
	bool keep_dimensions,
	bool ordered
)
	: m_reduction_count(0)
{
	if (input_signatures.empty())
	{
		throw std::invalid_argument(
			"reduction_layout_plan: A reduction needs at least one input "
			"operand to reduce."
		);
	}

	const auto extents = broadcast_input_extents(input_signatures);
	const auto rank = extents.size();

	std::vector<bool> is_reduced(rank, false);
	for (const auto axis : axes)
	{
		if (axis >= rank)
		{
			std::ostringstream oss;
			oss << "reduction_layout_plan: Axis " << axis
				<< " is out of range for a rank of " << rank << ".";
			throw std::out_of_range(oss.str());
		}
		is_reduced[axis] = true;
	}

	std::vector<std::size_t> kept_axes;
	std::vector<std::size_t> reduced_axes;
	for (std::size_t i = 0; i < rank; ++i)
	{
		if (is_reduced[i])
		{
			reduced_axes.push_back(i);
		}
		else
		{
			kept_axes.push_back(i);
		}
	}

	// A layout varies its first axis fastest, so counting positions the way
	// an index into the reduced space counts them means listing the reduced
	// axes last one first.
	if (ordered)
	{
		std::reverse(reduced_axes.begin(), reduced_axes.end());
	}

	const auto kept_extents = select_axes(extents, kept_axes);
	const auto reduced_extents = select_axes(extents, reduced_axes);

	joint_layout_builder kept_builder;
	joint_layout_builder reduced_builder;
	kept_builder.set_extents(make_span(kept_extents));
	reduced_builder.set_extents(make_span(reduced_extents));

	// The inputs come first so that they, and not the outputs, decide the
	// axis ordering: a reduction walks its inputs once per folded element for
	// every single output write.
	std::vector<std::ptrdiff_t> strides;
	for (const auto &signature : input_signatures)
	{
		const auto layout =
			signature.get_layout().broadcast_to(make_span(extents));
		layout.get_strides(strides);

		const auto kept_strides = select_axes(strides, kept_axes);
		kept_builder.add_operand(
			make_span(kept_extents),
			make_span(kept_strides),
			layout.get_offset()
		);

		// The offset belongs to the kept layout alone. The reduced cursor
		// displaces the pointer the kept one has already placed.
		const auto reduced_strides = select_axes(strides, reduced_axes);
		reduced_builder.add_operand(
			make_span(reduced_extents),
			make_span(reduced_strides),
			0
		);
	}

	for (const auto &signature : output_signatures)
	{
		const auto &layout = signature.get_layout();
		const auto output_strides = map_output_strides(
			layout,
			kept_axes,
			rank,
			keep_dimensions
		);

		kept_builder.add_operand(
			make_span(kept_extents),
			make_span(output_strides),
			layout.get_offset()
		);
	}

	// Merging adjacent axes leaves the order of the positions untouched;
	// sorting them for locality does not, so an ordered traversal keeps only
	// the former.
	REX_CONST_CONSTEXPR joint_layout_build_flags ordered_flags = {
		joint_layout_build_flag_bits::enable_coalescing
	};

	m_kept_layout = kept_builder.build();
	m_reduced_layout = ordered
		? reduced_builder.build(ordered_flags)
		: reduced_builder.build();
	m_reduction_count = element_count(reduced_extents);
}

const joint_layout&
reduction_layout_plan::get_kept_layout() const noexcept
{
	return m_kept_layout;
}

const joint_layout&
reduction_layout_plan::get_reduced_layout() const noexcept
{
	return m_reduced_layout;
}

std::size_t
reduction_layout_plan::get_reduction_count() const noexcept
{
	return m_reduction_count;
}

} // namespace cpu
} // namespace rex
