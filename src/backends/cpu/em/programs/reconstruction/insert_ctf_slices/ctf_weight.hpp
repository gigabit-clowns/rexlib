// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/em/optics/contrast_transfer_function.hpp>
#include <backends/cpu/loops/layouts/operand_core.hpp>

#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/em/ops/parameters/ctf_optics.hpp>
#include <rexlib/em/ops/reconstruction/insert_ctf_slices_operation.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The sample weights of insert_ctf_slices: the CTF c(k) for the
 * numerator and c(k)^2 for the denominator.
 *
 * @see unit_weight
 * @see em::insert_ctf_slices_operation
 */
class ctf_weight
{
public:
	/**
	 * @brief Parameters of the operation: the optics and the pixel size.
	 */
	class parameters_type
	{
	public:
		/**
		 * @brief Construct the parameters.
		 *
		 * @param optics The optics of the microscope.
		 * @param pixel_size The size of a pixel, in angstroms.
		 */
		parameters_type(
			const em::ctf_optics &optics,
			double pixel_size
		) noexcept;

		const em::ctf_optics& get_optics() const noexcept;
		double get_pixel_size() const noexcept;

	private:
		em::ctf_optics m_optics;
		double m_pixel_size;
	};

	/**
	 * @brief The CTF of one slice.
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
		 * @param ctf The four CTF parameters of the slice.
		 * @param ctf_core The core of the CTF parameters.
		 */
		slice_state(
			const parameters_type &parameters,
			std::size_t extent,
			const T *ctf,
			const operand_core &ctf_core
		) noexcept;

		/**
		 * @brief Get the CTF of the slice.
		 *
		 * @return const contrast_transfer_function<T>& The CTF.
		 */
		const contrast_transfer_function<T>& get_function() const noexcept;

	private:
		contrast_transfer_function<T> m_function;
	};

	/**
	 * @brief Index of the transforms among the inputs.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t transform_index = 3;

	/**
	 * @brief Index of the radial weights among the inputs.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t radial_index = 5;

	/**
	 * @brief Index of the CTF parameters among the inputs.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t extra_index = 4;

	/**
	 * @brief Get the parameters of an operation.
	 *
	 * @param operation The operation.
	 * @return parameters_type The parameters.
	 */
	static parameters_type make_parameters(
		const em::insert_ctf_slices_operation &operation
	) noexcept;

	/**
	 * @brief Fill the weights of a run of samples of one row of a slice.
	 *
	 * The CTF is even, so a sample and its Hermitian partner share it.
	 *
	 * @tparam T The real type of the weights.
	 * @param state The state of the slice.
	 * @param stored_row The stored index of the row.
	 * @param k_y The frequency of the row along y.
	 * @param k_begin The first sample.
	 * @param count The number of samples. Below 2^31.
	 * @param numerator_weights Receives c(k) for each sample.
	 * @param denominator_weights Receives c(k)^2 for each sample.
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

#include "ctf_weight.inl"
