// SPDX-License-Identifier: GPL-3.0-only

#include "spectrum_weight.hpp"

namespace rexlib
{
namespace cpu
{

template <typename T>
inline
spectrum_weight::slice_state<T>::slice_state() noexcept
	: m_weights(nullptr)
	, m_row_stride(0)
	, m_column_stride(0)
{
}

template <typename T>
inline
spectrum_weight::slice_state<T>::slice_state(
	const parameters_type&,
	std::size_t,
	const T *weights,
	const operand_core &weights_core
) noexcept
	: m_weights(weights)
	, m_row_stride(weights_core.get_stride(0))
	, m_column_stride(weights_core.get_stride(1))
{
}

template <typename T>
inline
T spectrum_weight::slice_state<T>::get_weight(
	std::ptrdiff_t stored_row,
	std::ptrdiff_t column
) const noexcept
{
	return m_weights[stored_row * m_row_stride + column * m_column_stride];
}

inline
spectrum_weight::parameters_type spectrum_weight::make_parameters(
	const em::insert_weighted_slices_operation&
) noexcept
{
	return parameters_type();
}

template <typename T>
inline
void spectrum_weight::fill(
	const slice_state<T> &state,
	std::ptrdiff_t stored_row,
	double,
	std::ptrdiff_t k_begin,
	std::size_t count,
	T *numerator_weights,
	T *denominator_weights
) noexcept
{
	for (std::size_t i = 0; i < count; ++i)
	{
		numerator_weights[i] = T(1);
		denominator_weights[i] = state.get_weight(
			stored_row,
			k_begin + static_cast<std::ptrdiff_t>(i)
		);
	}
}

} // namespace cpu
} // namespace rexlib
