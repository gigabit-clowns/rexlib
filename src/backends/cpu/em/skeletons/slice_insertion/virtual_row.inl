// SPDX-License-Identifier: GPL-3.0-only

#include "virtual_row.hpp"

namespace rexlib
{
namespace cpu
{

template <typename T>
inline
virtual_row<T>::virtual_row(
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
) noexcept
	: m_negated(negated)
	, m_stored_row(stored_row)
	, m_k_y(k_y)
	, m_base(base)
	, m_step(step)
	, m_samples(samples)
	, m_sample_stride(sample_stride)
	, m_x_phases(x_phases)
	, m_y_phase(y_phase)
	, m_radial_weights(radial_weights)
	, m_radial_stride(radial_stride)
	, m_last_shell(last_shell)
{
}

template <typename T>
inline
bool virtual_row<T>::is_negated() const noexcept
{
	return m_negated;
}

template <typename T>
inline
std::ptrdiff_t virtual_row<T>::get_stored_row() const noexcept
{
	return m_stored_row;
}

template <typename T>
inline
double virtual_row<T>::get_k_y() const noexcept
{
	return m_k_y;
}

template <typename T>
inline
const volume_frequency& virtual_row<T>::get_base() const noexcept
{
	return m_base;
}

template <typename T>
inline
const volume_frequency& virtual_row<T>::get_step() const noexcept
{
	return m_step;
}

template <typename T>
inline
const typename virtual_row<T>::complex_type*
virtual_row<T>::get_samples() const noexcept
{
	return m_samples;
}

template <typename T>
inline
std::ptrdiff_t virtual_row<T>::get_sample_stride() const noexcept
{
	return m_sample_stride;
}

template <typename T>
inline
const typename virtual_row<T>::complex_type*
virtual_row<T>::get_x_phases() const noexcept
{
	return m_x_phases;
}

template <typename T>
inline
typename virtual_row<T>::complex_type
virtual_row<T>::get_y_phase() const noexcept
{
	return m_y_phase;
}

template <typename T>
inline
const T* virtual_row<T>::get_radial_weights() const noexcept
{
	return m_radial_weights;
}

template <typename T>
inline
std::ptrdiff_t virtual_row<T>::get_radial_stride() const noexcept
{
	return m_radial_stride;
}

template <typename T>
inline
std::ptrdiff_t virtual_row<T>::get_last_shell() const noexcept
{
	return m_last_shell;
}

} // namespace cpu
} // namespace rexlib
