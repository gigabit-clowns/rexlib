// SPDX-License-Identifier: GPL-3.0-only

#include "insertion_chunk.hpp"

namespace rexlib
{
namespace cpu
{

template <typename T>
inline
typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_coordinates(std::size_t axis) noexcept
{
	return m_coordinates[axis];
}

template <typename T>
inline
const typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_coordinates(std::size_t axis) const noexcept
{
	return m_coordinates[axis];
}

template <typename T>
inline
typename insertion_chunk<T>::index_array_type&
insertion_chunk<T>::get_first_voxels(std::size_t axis) noexcept
{
	return m_first_voxels[axis];
}

template <typename T>
inline
const typename insertion_chunk<T>::index_array_type&
insertion_chunk<T>::get_first_voxels(std::size_t axis) const noexcept
{
	return m_first_voxels[axis];
}

template <typename T>
inline
typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_numerator_weights() noexcept
{
	return m_numerator_weights;
}

template <typename T>
inline
const typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_numerator_weights() const noexcept
{
	return m_numerator_weights;
}

template <typename T>
inline
typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_denominator_weights() noexcept
{
	return m_denominator_weights;
}

template <typename T>
inline
const typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_denominator_weights() const noexcept
{
	return m_denominator_weights;
}

template <typename T>
inline
typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_radial_weights() noexcept
{
	return m_radial_weights;
}

template <typename T>
inline
const typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_radial_weights() const noexcept
{
	return m_radial_weights;
}

template <typename T>
inline
typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_values_real() noexcept
{
	return m_values_real;
}

template <typename T>
inline
const typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_values_real() const noexcept
{
	return m_values_real;
}

template <typename T>
inline
typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_values_imaginary() noexcept
{
	return m_values_imaginary;
}

template <typename T>
inline
const typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_values_imaginary() const noexcept
{
	return m_values_imaginary;
}

template <typename T>
inline
typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_counts() noexcept
{
	return m_counts;
}

template <typename T>
inline
const typename insertion_chunk<T>::value_array_type&
insertion_chunk<T>::get_counts() const noexcept
{
	return m_counts;
}

} // namespace cpu
} // namespace rexlib
