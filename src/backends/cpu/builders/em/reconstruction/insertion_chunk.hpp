// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/constexpr.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

namespace rexlib
{
namespace cpu
{

/**
 * @brief What the first pass of a row insertion computes for a chunk of
 * samples, one array per quantity.
 *
 * Meant to live on the stack of the inserting function: no pointer it is
 * handed can reach an object there, which lets the loops filling it
 * vectorize without checking for overlap.
 *
 * @tparam T The real type of the accumulators.
 */
template <typename T>
class insertion_chunk
{
public:
	/**
	 * @brief Number of samples a chunk holds.
	 */
	static REXLIB_CONST_CONSTEXPR std::size_t size = 64;

	using value_array_type = std::array<T, size>;
	using index_array_type = std::array<std::int32_t, size>;

	value_array_type& get_coordinates(std::size_t axis) noexcept;
	const value_array_type& get_coordinates(std::size_t axis) const noexcept;
	index_array_type& get_first_voxels(std::size_t axis) noexcept;
	const index_array_type& get_first_voxels(std::size_t axis) const noexcept;
	value_array_type& get_numerator_weights() noexcept;
	const value_array_type& get_numerator_weights() const noexcept;
	value_array_type& get_denominator_weights() noexcept;
	const value_array_type& get_denominator_weights() const noexcept;
	value_array_type& get_radial_weights() noexcept;
	const value_array_type& get_radial_weights() const noexcept;
	value_array_type& get_values_real() noexcept;
	const value_array_type& get_values_real() const noexcept;
	value_array_type& get_values_imaginary() noexcept;
	const value_array_type& get_values_imaginary() const noexcept;
	value_array_type& get_counts() noexcept;
	const value_array_type& get_counts() const noexcept;

private:
	std::array<value_array_type, 3> m_coordinates;
	std::array<index_array_type, 3> m_first_voxels;
	value_array_type m_numerator_weights;
	value_array_type m_denominator_weights;
	value_array_type m_radial_weights;
	value_array_type m_values_real;
	value_array_type m_values_imaginary;
	value_array_type m_counts;
};

} // namespace cpu
} // namespace rexlib

#include "insertion_chunk.inl"
