// SPDX-License-Identifier: GPL-3.0-only

#include "fourier_program_builder.hpp"

#include <rexlib/core/dispatch/operand_signature.hpp>
#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/platform/cpp_attributes.hpp>

#include <backends/cpu/loops/loop_schedule.hpp>
#include <backends/cpu/loops/parallel_grain.hpp>

#include <tuple>
#include <utility>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Adapts a Fourier transform to the functor_program interface.
 *
 * Stores the transform and the planned iteration, and on invocation displaces
 * each operand pointer by its offset and hands the two over. The offsets are
 * applied here, once and in one place, so that everything downstream describes
 * an array starting where it is pointed at.
 */
template <typename Transform, typename Outputs, typename Inputs>
class fourier_transform_functor;

template <typename Transform, typename Out, typename In>
class fourier_transform_functor<Transform, type_list<Out>, type_list<In>>
{
public:
	fourier_transform_functor(Transform transform, fourier_layout_plan plan)
		: m_transform(std::move(transform))
		, m_plan(std::move(plan))
	{
	}

	void operator()(
		std::tuple<Out*> outputs,
		std::tuple<const In*> inputs,
		std::tuple<>,
		thread_pool &pool
	) const
	{
		// pocketfft is handed a count rather than this pool, threading a
		// transform from a pool of its own. The count is what it may spread a
		// transform over, and is ignored where that threading is off. The
		// grain is stated per element for the conversion pass, which is an
		// elementwise loop and the only part of this that uses the pool.
		m_transform(
			m_plan,
			std::get<0>(outputs) + m_plan.get_output_offset(),
			std::get<0>(inputs) + m_plan.get_input_offset(),
			loop_schedule(pool, grain_for_cost(1))
		);
	}

private:
	REXLIB_NO_UNIQUE_ADDRESS Transform m_transform;
	fourier_layout_plan m_plan;
};

template <typename Op, typename Transform, typename TypeDispatcher>
fourier_layout_plan
fourier_program_builder<Op, Transform, TypeDispatcher>::make_plan(
	const Op &operation,
	span<const operand_signature> output_signatures,
	span<const operand_signature> input_signatures
) const
{
	return fourier_layout_plan(
		output_signatures[0],
		input_signatures[0],
		operation.get_shape_policy().get_axes(),
		Transform::get_kind()
	);
}

template <typename Op, typename Transform, typename TypeDispatcher>
template <typename... Outs, typename... Ins>
auto
fourier_program_builder<Op, Transform, TypeDispatcher>::make_loop_functor(
	const Op &operation,
	fourier_layout_plan &plan,
	type_list<Outs...> /*output_element_types*/,
	type_list<Ins...> /*input_element_types*/
) const
{
	// The convention is a parameter of the operation and the direction is
	// baked into the transform's own type, so between the two the scale is
	// settled before a single element is read.
	Transform transform(operation.get_normalization());

	return fourier_transform_functor<
		Transform,
		type_list<Outs...>,
		type_list<Ins...>
	>(std::move(transform), std::move(plan));
}

} // namespace cpu
} // namespace rexlib
