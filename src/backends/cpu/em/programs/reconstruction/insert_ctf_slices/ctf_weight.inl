// SPDX-License-Identifier: GPL-3.0-only

#include "ctf_weight.hpp"

#include <cmath>
#include <cstdint>

namespace rexlib
{
namespace cpu
{

inline
ctf_weight::parameters_type::parameters_type(
	const em::ctf_optics &optics,
	double pixel_size
) noexcept
	: m_optics(optics)
	, m_pixel_size(pixel_size)
{
}

inline
const em::ctf_optics&
ctf_weight::parameters_type::get_optics() const noexcept
{
	return m_optics;
}

inline
double ctf_weight::parameters_type::get_pixel_size() const noexcept
{
	return m_pixel_size;
}

template <typename T>
inline
ctf_weight::slice_state<T>::slice_state(
	const parameters_type &parameters,
	std::size_t extent,
	const T *ctf,
	const operand_core &ctf_core
) noexcept
	: m_function(
		parameters.get_optics(),
		parameters.get_pixel_size(),
		extent,
		static_cast<double>(ctf[0]),
		static_cast<double>(ctf[ctf_core.get_stride(0)]),
		static_cast<double>(ctf[2 * ctf_core.get_stride(0)]),
		static_cast<double>(ctf[3 * ctf_core.get_stride(0)])
	)
{
}

template <typename T>
inline
const contrast_transfer_function<T>&
ctf_weight::slice_state<T>::get_function() const noexcept
{
	return m_function;
}

inline
ctf_weight::parameters_type ctf_weight::make_parameters(
	const em::insert_ctf_slices_operation &operation
) noexcept
{
	return parameters_type(operation.get_optics(), operation.get_pixel_size());
}

template <typename T>
inline
void ctf_weight::fill(
	const slice_state<T> &state,
	std::ptrdiff_t,
	double k_y,
	std::ptrdiff_t k_begin,
	std::size_t count,
	T *numerator_weights,
	T *denominator_weights
) noexcept
{
	// The phase and the square vectorize; the sine, without a vector
	// variant, runs alone between them.
	const auto function = state.get_function();
	const auto first = static_cast<T>(k_begin);
	const auto row = static_cast<T>(k_y);
	const auto signed_count = static_cast<std::int32_t>(count);
	for (std::int32_t i = 0; i < signed_count; ++i)
	{
		numerator_weights[i] =
			function.compute_phase(first + static_cast<T>(i), row);
	}
	for (std::size_t i = 0; i < count; ++i)
	{
		numerator_weights[i] = std::sin(numerator_weights[i]);
	}
	for (std::size_t i = 0; i < count; ++i)
	{
		denominator_weights[i] = numerator_weights[i] * numerator_weights[i];
	}
}

} // namespace cpu
} // namespace rexlib
