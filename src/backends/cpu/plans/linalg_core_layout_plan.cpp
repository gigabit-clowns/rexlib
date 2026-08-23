// SPDX-License-Identifier: GPL-3.0-only

#include "linalg_core_layout_plan.hpp"

#include <xmipp4/core/layout/joint_layout_builder.hpp>
#include <xmipp4/core/layout/strided_layout.hpp>

#include <array>
#include <stdexcept>
#include <utility>
#include <vector>

namespace xmipp4
{
namespace cpu
{

namespace
{

/**
 * @brief Split a layout's trailing axes off as a core, leaving the rest as
 * the batch prefix.
 */
linalg_operand_core split_trailing_core(
	const strided_layout &layout,
	std::size_t core_rank,
	std::vector<std::size_t> &batch_extents,
	std::vector<std::ptrdiff_t> &batch_strides
)
{
	std::vector<std::size_t> extents;
	std::vector<std::ptrdiff_t> strides;
	layout.get_extents(extents);
	layout.get_strides(strides);

	if (extents.size() < core_rank)
	{
		throw std::invalid_argument(
			"linalg_core_layout_plan: operand does not have enough axes "
			"for its core."
		);
	}

	const auto batch_rank = extents.size() - core_rank;
	batch_extents.assign(extents.begin(), extents.begin() + batch_rank);
	batch_strides.assign(strides.begin(), strides.begin() + batch_rank);

	std::array<std::size_t, 2> core_extents{};
	std::array<std::ptrdiff_t, 2> core_strides{};
	for (std::size_t i = 0; i < core_rank; ++i)
	{
		core_extents[i] = extents[batch_rank + i];
		core_strides[i] = strides[batch_rank + i];
	}
	return linalg_operand_core(core_rank, core_extents, core_strides);
}

/**
 * @brief Broadcast a layout to the canonical extents and split the named
 * axis off as a core, leaving the rest, in their original relative order,
 * as the batch.
 */
linalg_operand_core split_named_core(
	const strided_layout &layout,
	span<const std::size_t> canonical_extents,
	std::size_t core_axis,
	std::vector<std::size_t> &batch_extents,
	std::vector<std::ptrdiff_t> &batch_strides,
	std::ptrdiff_t &offset
)
{
	const auto broadcast_layout = layout.broadcast_to(canonical_extents);

	std::vector<std::size_t> extents;
	std::vector<std::ptrdiff_t> strides;
	broadcast_layout.get_extents(extents);
	broadcast_layout.get_strides(strides);
	offset = broadcast_layout.get_offset();

	const std::array<std::size_t, 2> core_extents{ extents.at(core_axis), 0 };
	const std::array<std::ptrdiff_t, 2> core_strides{
		strides.at(core_axis), 0
	};

	batch_extents.clear();
	batch_strides.clear();
	for (std::size_t i = 0; i < extents.size(); ++i)
	{
		if (i != core_axis)
		{
			batch_extents.push_back(extents[i]);
			batch_strides.push_back(strides[i]);
		}
	}
	return linalg_operand_core(1, core_extents, core_strides);
}

} // anonymous namespace

linalg_core_layout_plan::linalg_core_layout_plan(
	joint_layout batch_layout,
	const linalg_operand_core &output_core,
	const linalg_operand_core &left_core,
	const linalg_operand_core &right_core
) noexcept
	: m_batch_layout(std::move(batch_layout))
	, m_output_core(output_core)
	, m_left_core(left_core)
	, m_right_core(right_core)
{
}

linalg_core_layout_plan linalg_core_layout_plan::for_trailing_core(
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures,
	std::size_t output_core_rank,
	std::size_t left_core_rank,
	std::size_t right_core_rank
)
{
	const auto &output_layout = output_signatures[0].get_layout();
	const auto &left_layout = input_signatures[0].get_layout();
	const auto &right_layout = input_signatures[1].get_layout();

	joint_layout_builder builder;
	std::vector<std::size_t> batch_extents;
	std::vector<std::ptrdiff_t> batch_strides;

	const auto output_core = split_trailing_core(
		output_layout, output_core_rank, batch_extents, batch_strides
	);
	builder.add_operand(
		make_span(batch_extents),
		make_span(batch_strides),
		output_layout.get_offset()
	);

	const auto left_core = split_trailing_core(
		left_layout, left_core_rank, batch_extents, batch_strides
	);
	builder.add_operand(
		make_span(batch_extents),
		make_span(batch_strides),
		left_layout.get_offset()
	);

	const auto right_core = split_trailing_core(
		right_layout, right_core_rank, batch_extents, batch_strides
	);
	builder.add_operand(
		make_span(batch_extents),
		make_span(batch_strides),
		right_layout.get_offset()
	);

	return linalg_core_layout_plan(
		builder.build(), output_core, left_core, right_core
	);
}

linalg_core_layout_plan linalg_core_layout_plan::for_named_axis(
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures,
	std::size_t core_axis
)
{
	std::vector<std::size_t> canonical_extents;
	output_signatures[0].get_layout().get_extents(canonical_extents);

	joint_layout_builder builder;
	std::vector<std::size_t> batch_extents;
	std::vector<std::ptrdiff_t> batch_strides;
	std::ptrdiff_t offset;

	const auto output_core = split_named_core(
		output_signatures[0].get_layout(),
		make_span(canonical_extents),
		core_axis,
		batch_extents, batch_strides, offset
	);
	builder.add_operand(make_span(batch_extents), make_span(batch_strides), offset);

	const auto left_core = split_named_core(
		input_signatures[0].get_layout(),
		make_span(canonical_extents),
		core_axis,
		batch_extents, batch_strides, offset
	);
	builder.add_operand(make_span(batch_extents), make_span(batch_strides), offset);

	const auto right_core = split_named_core(
		input_signatures[1].get_layout(),
		make_span(canonical_extents),
		core_axis,
		batch_extents, batch_strides, offset
	);
	builder.add_operand(make_span(batch_extents), make_span(batch_strides), offset);

	return linalg_core_layout_plan(
		builder.build(), output_core, left_core, right_core
	);
}

const joint_layout& linalg_core_layout_plan::get_batch_layout() const noexcept
{
	return m_batch_layout;
}

std::size_t linalg_core_layout_plan::get_core_cost() const noexcept
{
	std::size_t result = 1;
	for (std::size_t i = 0; i < m_left_core.get_rank(); ++i)
	{
		result *= m_left_core.get_extent(i);
	}

	// Only a matrix on the right widens the product. A vector there is the
	// matvec case, whose cost the left operand's extents already state.
	if (m_right_core.get_rank() == 2)
	{
		result *= m_right_core.get_extent(1);
	}

	return result > 0 ? result : 1;
}

const linalg_operand_core& 
linalg_core_layout_plan::get_output_core() const noexcept
{
	return m_output_core;
}

const linalg_operand_core& 
linalg_core_layout_plan::get_left_core() const noexcept
{
	return m_left_core;
}

const linalg_operand_core& 
linalg_core_layout_plan::get_right_core() const noexcept
{
	return m_right_core;
}

} // namespace cpu
} // namespace xmipp4
