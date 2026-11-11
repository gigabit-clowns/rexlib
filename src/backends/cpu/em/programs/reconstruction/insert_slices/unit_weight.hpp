// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/layouts/operand_core.hpp>

#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/em/ops/reconstruction/insert_slices_operation.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The sample weights of insert_slices: 1 for the numerator and 1
 * for the denominator.
 *
 * A sample weight policy names the inputs of its operation beyond the
 * accumulators and the slices, holds the parameters its operation carries,
 * builds a state per slice, and fills the weights of a run of samples.
 *
 * @see em::insert_slices_operation
 */
class unit_weight
{
public:
	/**
	 * @brief Parameters of the operation. None.
	 */
	class parameters_type
	{
	};

	/**
	 * @brief The state of one slice. None.
	 *
	 * @tparam T The real type of the weights.
	 */
	template <typename T>
	class slice_state
	{
	public:
		slice_state() noexcept = default;

		/**
		 * @brief Build the state of a slice.
		 *
		 * @param parameters The parameters of the operation.
		 * @param extent The extent of a slice.
		 * @param extra The extra input of the operation at this slice.
		 * @param extra_core The core of the extra input.
		 */
		slice_state(
			const parameters_type &parameters,
			std::size_t extent,
			const T *extra,
			const operand_core &extra_core
		) noexcept;
	};

	/**
	 * @brief Index of the transforms among the inputs.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t transform_index = 3;

	/**
	 * @brief Index of the radial weights among the inputs.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t radial_index = 4;

	/**
	 * @brief Index of the extra input, which this policy lacks: the radial
	 * weights stand in.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t extra_index = 4;

	/**
	 * @brief Get the parameters of an operation.
	 *
	 * @param operation The operation.
	 * @return parameters_type The parameters.
	 */
	static parameters_type make_parameters(
		const em::insert_slices_operation &operation
	) noexcept;

	/**
	 * @brief Fill the weights of a run of samples of one row of a slice.
	 *
	 * @tparam T The real type of the weights.
	 * @param state The state of the slice.
	 * @param stored_row The stored index of the row.
	 * @param k_y The frequency of the row along y.
	 * @param k_begin The first sample.
	 * @param count The number of samples.
	 * @param numerator_weights Receives the weight of each sample in the
	 * numerator.
	 * @param denominator_weights Receives the weight of each sample in the
	 * denominator.
	 */
	template <typename T>
	static void fill(
		const slice_state<T> &state,
		std::ptrdiff_t stored_row,
		double k_y,
		std::ptrdiff_t k_begin,
		std::size_t count,
		T *numerator_weights,
		T *denominator_weights
	) noexcept;
};

} // namespace cpu
} // namespace rexlib

#include "unit_weight.inl"
