// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "virtual_row.hpp"

#include <backends/cpu/kernels/frequency_bounds.hpp>
#include <backends/cpu/plans/frequency_interval.hpp>
#include <backends/cpu/plans/operand_core.hpp>

#include <rexlib/core/platform/constexpr.hpp>

#include <complex>
#include <cstddef>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Inserts the samples of a row of a slice into a box of a volume.
 *
 * The box is a range of frequencies along z and along y, with every
 * frequency along x from 0 to half the extent. A run of samples is
 * processed in chunks: a first pass over a chunk computes where each
 * sample lands, the weights of the voxels it reaches and the value it
 * carries; a second pass adds the values into the voxels within the box.
 *
 * @tparam T The real type of the accumulators.
 * @tparam Window The window spreading a sample over the voxels of an axis.
 * @tparam VolumePhase The phase a voxel carries.
 * @tparam SampleWeight The weights of a sample in the accumulators.
 */
template <
	typename T,
	typename Window,
	typename VolumePhase,
	typename SampleWeight
>
class row_insertion_kernel
{
public:
	using complex_type = std::complex<T>;
	using slice_state_type = typename SampleWeight::template slice_state<T>;

	/**
	 * @brief Number of samples a pass covers.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t chunk_size = 64;

	/**
	 * @brief Construct a kernel.
	 *
	 * @param window The window.
	 * @param phase The phase of the voxels.
	 * @param volume_extent The extent N of the volume.
	 * @param numerator_core The core of the numerator.
	 * @param denominator_core The core of the denominator.
	 */
	row_insertion_kernel(
		const Window &window,
		const VolumePhase &phase,
		std::size_t volume_extent,
		const operand_core &numerator_core,
		const operand_core &denominator_core
	);

	/**
	 * @brief Set the accumulators to insert into.
	 *
	 * @param numerator The numerator, at the voxel (0, 0, 0).
	 * @param denominator The denominator, at the voxel (0, 0, 0).
	 */
	void bind(complex_type *numerator, T *denominator) noexcept;

	/**
	 * @brief Set the box to insert into.
	 *
	 * @param z The frequencies of the box along z.
	 * @param y The frequencies of the box along y.
	 */
	void set_box(
		const frequency_interval &z,
		const frequency_interval &y
	) noexcept;

	/**
	 * @brief Get the frequencies a sample may land on and still reach the
	 * box.
	 *
	 * @return const frequency_bounds& The box, widened by the half-width of
	 * the window.
	 */
	const frequency_bounds& get_reach() const noexcept;

	/**
	 * @brief Insert a run of samples of a row.
	 *
	 * @param row The row.
	 * @param state The state of the slice under the sample weights.
	 * @param begin The first sample.
	 * @param end One past the last sample.
	 */
	void insert(
		const virtual_row<T> &row,
		const slice_state_type &state,
		std::ptrdiff_t begin,
		std::ptrdiff_t end
	);

private:
	void prepare(
		const virtual_row<T> &row,
		const slice_state_type &state,
		std::ptrdiff_t begin,
		std::size_t count
	);

	void scatter(std::size_t count) noexcept;

	std::ptrdiff_t stored_index(std::ptrdiff_t frequency) const noexcept;

	const Window *m_window;
	const VolumePhase *m_phase;
	std::ptrdiff_t m_extent;
	std::ptrdiff_t m_half_extent;
	std::size_t m_tap_count;
	operand_core m_numerator_core;
	operand_core m_denominator_core;
	complex_type *m_numerator;
	T *m_denominator;
	frequency_interval m_z;
	frequency_interval m_y;
	frequency_bounds m_reach;
	std::vector<std::ptrdiff_t> m_first_x;
	std::vector<std::ptrdiff_t> m_first_y;
	std::vector<std::ptrdiff_t> m_first_z;
	std::vector<T> m_weights_x;
	std::vector<T> m_weights_y;
	std::vector<T> m_weights_z;
	std::vector<T> m_numerator_weights;
	std::vector<T> m_denominator_weights;
	std::vector<complex_type> m_values;
	std::vector<T> m_counts;
	std::vector<unsigned char> m_active;
};

} // namespace cpu
} // namespace rexlib

#include "row_insertion_kernel.inl"
