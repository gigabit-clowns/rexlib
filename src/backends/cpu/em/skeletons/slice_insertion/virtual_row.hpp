// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/math/fourier/slice_frame.hpp>

#include <complex>
#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief One row of the full spectrum of a slice, read from a stored row
 * with a sign.
 *
 * The sample k of the row is the frequency sign * (k, k_y) of the slice,
 * which lands on base + k step in the volume. With a negative sign, the
 * stored value and the shift phase are conjugated.
 *
 * @tparam T The real type of the slice.
 */
template <typename T>
class virtual_row
{
public:
	using complex_type = std::complex<T>;

	/**
	 * @brief Construct a row.
	 *
	 * @param negated Whether the sign is negative.
	 * @param stored_row The stored index of the row.
	 * @param k_y The frequency of the stored row along y.
	 * @param base The volume frequency of the sample k = 0.
	 * @param step The step of the volume frequency per sample.
	 * @param samples The stored row, at k = 0.
	 * @param sample_stride The stride between samples, in elements.
	 * @param x_phases The shift phases along x, at k = 0.
	 * @param y_phase The shift phase of the row.
	 * @param radial_weights The radial weights of the slice, at shell 0.
	 * @param radial_stride The stride between shells, in elements.
	 * @param last_shell The last shell with a nonzero radial weight.
	 */
	virtual_row(
		bool negated,
		std::ptrdiff_t stored_row,
		double k_y,
		const volume_frequency &base,
		const volume_frequency &step,
		const complex_type *samples,
		std::ptrdiff_t sample_stride,
		const complex_type *x_phases,
		complex_type y_phase,
		const T *radial_weights,
		std::ptrdiff_t radial_stride,
		std::ptrdiff_t last_shell
	) noexcept;

	bool is_negated() const noexcept;
	std::ptrdiff_t get_stored_row() const noexcept;
	double get_k_y() const noexcept;
	const volume_frequency& get_base() const noexcept;
	const volume_frequency& get_step() const noexcept;
	const complex_type* get_samples() const noexcept;
	std::ptrdiff_t get_sample_stride() const noexcept;
	const complex_type* get_x_phases() const noexcept;
	complex_type get_y_phase() const noexcept;
	const T* get_radial_weights() const noexcept;
	std::ptrdiff_t get_radial_stride() const noexcept;
	std::ptrdiff_t get_last_shell() const noexcept;

private:
	bool m_negated;
	std::ptrdiff_t m_stored_row;
	double m_k_y;
	volume_frequency m_base;
	volume_frequency m_step;
	const complex_type *m_samples;
	std::ptrdiff_t m_sample_stride;
	const complex_type *m_x_phases;
	complex_type m_y_phase;
	const T *m_radial_weights;
	std::ptrdiff_t m_radial_stride;
	std::ptrdiff_t m_last_shell;
};

} // namespace cpu
} // namespace rexlib

#include "virtual_row.inl"
