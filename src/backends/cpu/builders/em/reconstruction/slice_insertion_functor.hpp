// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "slice_entry.hpp"
#include "slice_insertion_tuning.hpp"

#include <backends/cpu/plans/slice_insertion_layout_plan.hpp>
#include <backends/cpu/plans/volume_tile.hpp>

#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/em/ops/parameters/gridding_window.hpp>

#include <complex>
#include <cstddef>
#include <tuple>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Runs a slice insertion over its operands.
 *
 * A call first settles every slice: its frame, its phase tables and its
 * state under the sample weights. Then the tiles of every accumulator are
 * claimed by the threads of the pool, and each tile is copied from its
 * accumulator, unless the output is the accumulator itself, and receives
 * every slice.
 *
 * @tparam T The real type of the accumulators.
 * @tparam SampleWeight The weights of a sample in the accumulators.
 */
template <typename T, typename SampleWeight>
class slice_insertion_functor
{
public:
	using complex_type = std::complex<T>;
	using parameters_type = typename SampleWeight::parameters_type;
	using slice_state_type = typename SampleWeight::template slice_state<T>;

	/**
	 * @brief Construct the functor of an operation.
	 *
	 * @param plan The plan of the operands.
	 * @param window The gridding window.
	 * @param parameters The parameters of the sample weights.
	 * @param tuning The sizing of the tiles.
	 */
	slice_insertion_functor(
		slice_insertion_layout_plan plan,
		const em::gridding_window &window,
		const parameters_type &parameters,
		const slice_insertion_tuning &tuning
	);

	/**
	 * @brief Run the insertion.
	 *
	 * @param outputs The numerator and the denominator.
	 * @param inputs The accumulators, the slices and the rest.
	 * @param pool The pool to spread the work over.
	 */
	template <typename... Outs, typename... Ins>
	void operator()(
		const std::tuple<Outs*...> &outputs,
		const std::tuple<const Ins*...> &inputs,
		std::tuple<>,
		thread_pool &pool
	) const;

private:
	template <typename Window, typename... Outs, typename... Ins>
	void run_with_window(
		const Window &window,
		const std::tuple<Outs*...> &outputs,
		const std::tuple<const Ins*...> &inputs,
		thread_pool &pool
	) const;

	template <typename Window, typename VolumePhase, typename... Ins>
	void run(
		const Window &window,
		const VolumePhase &phase,
		complex_type *numerator,
		T *denominator,
		const std::tuple<const Ins*...> &inputs,
		thread_pool &pool
	) const;

	template <typename... Ins>
	void settle_slices(
		const std::tuple<const Ins*...> &inputs,
		std::vector<slice_entry> &entries,
		std::vector<slice_state_type> &states,
		std::vector<complex_type> &phases,
		thread_pool &pool
	) const;

	template <typename Window, typename VolumePhase, typename... Ins>
	void insert_tile(
		const Window &window,
		const VolumePhase &phase,
		complex_type *numerator,
		T *denominator,
		const std::tuple<const Ins*...> &inputs,
		const std::vector<slice_entry> &entries,
		const std::vector<slice_state_type> &states,
		const std::vector<complex_type> &phases,
		std::size_t kept,
		const volume_tile &tile
	) const;

	template <typename U>
	void copy_tile(
		U *output,
		const operand_core &output_core,
		const U *accumulator,
		const operand_core &accumulator_core,
		const volume_tile &tile
	) const noexcept;

	slice_insertion_layout_plan m_plan;
	em::gridding_window m_window;
	parameters_type m_parameters;
	slice_insertion_tuning m_tuning;
};

} // namespace cpu
} // namespace rexlib

#include "slice_insertion_functor.inl"
