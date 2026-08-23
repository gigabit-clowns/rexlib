// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/layout/dynamic_subscript.hpp>
#include <rex/core/span.hpp>

#include "strided_axis.hpp"
#include "../config.hpp"

#include <boost/container/small_vector.hpp>

namespace rex 
{

class strided_layout_implementation
{
public:
	using strided_axis_vector_type = boost::container::small_vector<
		strided_axis, 
		REX_SMALL_AXIS_COUNT
	>;

	strided_layout_implementation() noexcept;

	strided_layout_implementation(
		const strided_axis_vector_type &axes,
		std::ptrdiff_t offset 
	);

	strided_layout_implementation(
		strided_axis_vector_type &&axes,
		std::ptrdiff_t offset 
	) noexcept;

	friend bool operator==(
		const strided_layout_implementation &lhs,
		const strided_layout_implementation &rhs
	) noexcept
	{
		if (lhs.m_offset != rhs.m_offset)
		{
			return false;
		}

		if (lhs.m_axes.size() != rhs.m_axes.size())
		{
			return false;
		}

		const auto n = lhs.m_axes.size();
		for (std::size_t i = 0; i < n; ++i)
		{
			const auto &axis1 = lhs.m_axes[i];
			const auto &axis2 = rhs.m_axes[i];

			const auto extent1 = axis1.get_extent();
			const auto extent2 = axis2.get_extent();
			if (extent1 != extent2)
			{
				return false;
			}

			const auto stride1 = axis1.get_stride();
			const auto stride2 = axis2.get_stride();
			if (extent1 != 1 && stride1 != stride2)
			{
				return false;
			}
		}

		return true;
	}

	std::size_t hash() const noexcept;

	std::size_t get_rank() const noexcept;

	template <typename Vec>
	void get_extents(Vec &extents) const;
	template <typename Vec>
	void get_strides(Vec &strides) const;
	std::ptrdiff_t get_offset() const noexcept;
	std::size_t compute_storage_requirement() const noexcept;
	std::size_t compute_element_count() const noexcept;
	bool extents_equal(span<const std::size_t> extents) const noexcept;

	strided_layout_implementation
	apply_subscripts(span<const dynamic_subscript> subscripts) const;

	strided_layout_implementation transpose() const;

	strided_layout_implementation permute(span<const std::size_t> order) const;

	strided_layout_implementation matrix_transpose(
		std::ptrdiff_t axis1, 
		std::ptrdiff_t axis2
	) const;

	strided_layout_implementation matrix_diagonal(
		std::ptrdiff_t axis1, 
		std::ptrdiff_t axis2
	) const;

	strided_layout_implementation squeeze() const;


	strided_layout_implementation broadcast_to(
		span<const std::size_t> extents
	) const;

private:
	strided_axis_vector_type m_axes;
	std::ptrdiff_t m_offset;
};

} // namespace rex

#include "strided_layout_implementation.inl"
