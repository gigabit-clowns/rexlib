// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/layouts/operand_core.hpp>

#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/em/ops/reconstruction/insert_weighted_slices_operation.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The sample weights of insert_weighted_slices: 1 for the numerator
 * and the weight of the frequency for the denominator.
 *
 * @see unit_weight
 * @see em::insert_weighted_slices_operation
 */
class spectrum_weight
{
public:
	/**
	 * @brief Parameters of the operation. None.
	 */
	class parameters_type
	{
	};

	/**
	 * @brief The weights of one slice.
	 *
	 * @tparam T The real type of the weights.
	 */
	template <typename T>
	class slice_state
	{
	public:
		slice_state() noexcept;

		/**
		 * @brief Build the state of a slice.
		 *
		 * @param parameters The parameters of the operation.
		 * @param extent The extent of a slice.
		 * @param weights The weights of the slice, at the frequency 0.
		 * @param weights_core The core of the weights.
		 */
		slice_state(
			const parameters_type &parameters,
			std::size_t extent,
			const T *weights,
			const operand_core &weights_core
		) noexcept;

		/**
		 * @brief Get the weight of a stored element.
		 *
		 * @param stored_row The stored index of the row.
		 * @param column The stored index of the column.
		 * @return T The weight.
		 */
		T get_weight(
			std::ptrdiff_t stored_row,
			std::ptrdiff_t column
		) const noexcept;

	private:
		const T *m_weights;
		std::ptrdiff_t m_row_stride;
		std::ptrdiff_t m_column_stride;
	};

	/**
	 * @brief Index of the transforms among the inputs.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t transform_index = 4;

	/**
	 * @brief Index of the radial weights among the inputs.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t radial_index = 5;

	/**
	 * @brief Index of the weights among the inputs.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t extra_index = 3;

	/**
	 * @brief Get the parameters of an operation.
	 *
	 * @param operation The operation.
	 * @return parameters_type The parameters.
	 */
	static parameters_type make_parameters(
		const em::insert_weighted_slices_operation &operation
	) noexcept;

	/**
	 * @brief Fill the weights of a run of samples of one row of a slice.
	 *
	 * The sample k of a row stands for a stored element and, through its
	 * sign, for its Hermitian partner; both have the weight of the stored
	 * element.
	 *
	 * @tparam T The real type of the weights.
	 * @param state The state of the slice.
	 * @param stored_row The stored index of the row.
	 * @param k_y The frequency of the row along y.
	 * @param k_begin The first sample.
	 * @param count The number of samples.
	 * @param numerator_weights Receives 1 for each sample.
	 * @param denominator_weights Receives the weight of each sample.
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

#include "spectrum_weight.inl"
