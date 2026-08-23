// SPDX-License-Identifier: GPL-3.0-only

#include "roll_program_builder.hpp"

#include <rex/core/dispatch/operand_signature.hpp>
#include <rex/core/layout/strided_layout.hpp>
#include <rex/core/meta/type_list.hpp>

#include <backends/cpu/loops/elementwise_loop.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/loops/parallel_grain.hpp>
#include <backends/cpu/plans/roll_layout_plan.hpp>

#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

namespace rex
{
namespace cpu
{

/**
 * @brief Copy one element from an input pointer to an output one.
 *
 * A cyclic shift moves elements without converting them, and
 * unary_homogeneous_rule<> guarantees the output and input share their
 * element type, so there is nothing to do beyond the assignment.
 */
template <typename T>
struct roll_assign
{
	void operator()(T *out, const T *in) const noexcept
	{
		*out = *in;
	}
};

/**
 * @brief Per-element kernel for a cyclic axis shift.
 *
 * Drives run_elementwise_loop once per block precomputed by
 * build_roll_blocks, each a plain affine copy; the wraparound lives
 * entirely in how those blocks were carved out, not in this loop.
 */
template <typename T>
class roll_kernel
{
public:
	explicit roll_kernel(std::vector<joint_layout> blocks)
		: m_blocks(std::move(blocks))
	{
	}

	void operator()(
		std::tuple<T *> outputs,
		std::tuple<const T *> inputs,
		std::tuple<>,
		thread_pool &pool
	) const
	{
		// Inside each block rather than across them. A roll of k axes gives
		// 2^k blocks of very unequal size, so splitting the block loop would
		// leave one thread with almost all of the work; splitting inside lets
		// the one large block spread out while the slivers fall below the
		// grain and stay where they are.
		const loop_schedule schedule(pool, grain_for_cost(1));

		T *const out = std::get<0>(outputs);
		const T *const in = std::get<0>(inputs);
		for (const auto &block : m_blocks)
		{
			run_elementwise_loop(roll_assign<T>(), block, schedule, out, in);
		}
	}

private:
	std::vector<joint_layout> m_blocks;
};

template <typename Op, typename ShiftPolicy, typename TypeDispatcher>
std::vector<joint_layout>
roll_program_builder<Op, ShiftPolicy, TypeDispatcher>::make_plan(
	const Op &operation,
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures
) const
{
	const auto axes = operation.get_shape_policy().get_axes();

	const auto &output_layout = output_signatures[0].get_layout();
	const auto &input_layout = input_signatures[0].get_layout();

	std::vector<std::size_t> extents;
	output_layout.get_extents(extents);
	std::vector<std::ptrdiff_t> output_strides;
	output_layout.get_strides(output_strides);
	std::vector<std::ptrdiff_t> input_strides;
	input_layout.get_strides(input_strides);

	std::vector<roll_axis_shift> shifts;
	shifts.reserve(axes.size());
	for (const auto axis : axes)
	{
		const auto shift = ShiftPolicy::shift_amount(extents[axis]);
		if (shift > 0)
		{
			shifts.emplace_back(axis, shift);
		}
	}

	return build_roll_blocks(
		make_span(extents),
		make_span(output_strides),
		output_layout.get_offset(),
		make_span(input_strides),
		input_layout.get_offset(),
		make_span(shifts)
	);
}

template <typename Op, typename ShiftPolicy, typename TypeDispatcher>
template <typename... Outs, typename... Ins>
auto
roll_program_builder<Op, ShiftPolicy, TypeDispatcher>::make_loop_functor(
	const Op& /*operation*/,
	std::vector<joint_layout> &blocks,
	type_list<Outs...> output_element_types,
	type_list<Ins...> /*input_element_types*/
) const
{
	using out_t = typename type_list_element<
		0, decltype(output_element_types)
	>::type;

	return roll_kernel<out_t>(std::move(blocks));
}

} // namespace cpu
} // namespace rex
