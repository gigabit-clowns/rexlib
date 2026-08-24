// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>

#include "../config.hpp"

#include <vector>
#include <cstddef>

#include <boost/container/small_vector.hpp>

namespace rexlib 
{

class joint_layout_operand
{
public:
	using stride_vector_type = boost::container::small_vector<
		std::ptrdiff_t, 
		REXLIB_SMALL_AXIS_COUNT
	>;

	joint_layout_operand(
		const stride_vector_type &strides,
		std::ptrdiff_t offset
	);

	span<const std::ptrdiff_t> get_strides() const noexcept;

	std::ptrdiff_t get_offset() const noexcept;

	int compare_strides(std::size_t i, std::size_t j) const noexcept;

	void swap_axes(std::size_t i, std::size_t j) noexcept;

	void trim_axes(std::size_t n);

private:
	stride_vector_type m_strides;
	std::ptrdiff_t m_offset;
};

} // namespace rexlib

#include "joint_layout_operand.inl"
