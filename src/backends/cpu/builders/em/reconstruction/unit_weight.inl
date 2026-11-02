// SPDX-License-Identifier: GPL-3.0-only

#include "unit_weight.hpp"

namespace rexlib
{
namespace cpu
{

template <typename T>
inline
unit_weight::slice_state<T>::slice_state(
	const parameters_type&,
	std::size_t,
	const T*,
	const operand_core&
) noexcept
{
}

inline
unit_weight::parameters_type unit_weight::make_parameters(
	const em::insert_slices_operation&
) noexcept
{
	return parameters_type();
}

template <typename T>
inline
void unit_weight::fill(
	const slice_state<T>&,
	std::ptrdiff_t,
	double,
	std::ptrdiff_t,
	std::size_t count,
	T *numerator_weights,
	T *denominator_weights
) noexcept
{
	for (std::size_t i = 0; i < count; ++i)
	{
		numerator_weights[i] = T(1);
		denominator_weights[i] = T(1);
	}
}

} // namespace cpu
} // namespace rexlib
