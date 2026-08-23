// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/layout/joint_layout_builder.hpp>

#include <xmipp4/core/layout/strided_layout.hpp>
#include <xmipp4/core/exceptions/invalid_operation_error.hpp>

#include "joint_layout_implementation.hpp"
#include "strided_layout_implementation.hpp"

namespace rex 
{

namespace
{

void broadcast_axis_to(
	std::size_t &extent, 
	std::ptrdiff_t &stride, 
	std::size_t target_extent
)
{
	if (extent != target_extent)
	{
		if (extent == 1)
		{
			extent = target_extent;
			stride = 0;
		}
		else
		{
			std::ostringstream oss;
			oss << "Unable to broadcast extent of " << extent
				<< " into target extent of " << target_extent << ".";
			throw std::invalid_argument(oss.str());
		}
	}
}

void broadcast_to(
	joint_layout_implementation::extent_vector_type &extents,
	joint_layout_implementation::stride_vector_type &strides,
	span<const std::size_t> target_extents
)
{
	REX_ASSERT( extents.size() == strides.size() );

	const auto n = target_extents.size();
	if (extents.size() < n )
	{
		
		const auto padding = target_extents.size() - extents.size();
		extents.insert(extents.cbegin(), padding, 1UL);
		strides.insert(strides.cbegin(), padding, 0UL);
	}

	REX_ASSERT( extents.size() == n );
	REX_ASSERT( strides.size() == n );

	for (std::size_t i = 0; i < n; ++i)
	{
		broadcast_axis_to(extents[i], strides[i], target_extents[i]);
	}
}

void create_or_add_operand(
	std::unique_ptr<joint_layout_implementation> &implementation,
	joint_layout_implementation::extent_vector_type &&extents,
	joint_layout_implementation::stride_vector_type &&strides,
	std::ptrdiff_t offset
)
{
	if (implementation)
	{
		broadcast_to(
			extents,
			strides,
			implementation->get_extents()
		);
	}
	else
	{
		implementation = 
			std::make_unique<joint_layout_implementation>(
				std::move(extents)
			);

	}

	REX_ASSERT( implementation );
	implementation->add_operand(std::move(strides), offset);
}

}

joint_layout_builder::joint_layout_builder(
) noexcept = default;

joint_layout_builder::joint_layout_builder(
	joint_layout_builder&& other
) noexcept = default;

joint_layout_builder::~joint_layout_builder(
) = default;

joint_layout_builder& 
joint_layout_builder::operator=(
	joint_layout_builder&& other
) noexcept = default;

joint_layout_builder& 
joint_layout_builder::set_extents(span<const std::size_t> extents)
{
	if (m_implementation)
	{
		throw invalid_operation_error(
			"Extents can only be set once and before adding any operand"
		);
	}

	m_implementation = 
		std::make_unique<joint_layout_implementation>(
			joint_layout_implementation::extent_vector_type(
				extents.begin(),
				extents.end()
			)
		);

	return *this;
}

joint_layout_builder& 
joint_layout_builder::add_operand(
	span<const std::size_t> extents,
	span<const std::ptrdiff_t> strides,
	std::ptrdiff_t offset
)
{
	create_or_add_operand(
		m_implementation,
		joint_layout_implementation::extent_vector_type(
			extents.begin(),
			extents.end()
		),
		joint_layout_implementation::stride_vector_type(
			strides.begin(),
			strides.end()
		),
		offset
	);

	return *this;
}

joint_layout_builder& 
joint_layout_builder::add_operand(const strided_layout &layout)
{

	const auto &layout_impl = layout.get_implementation();


	joint_layout_implementation::extent_vector_type extents;
	layout_impl.get_extents(extents);

	joint_layout_implementation::stride_vector_type strides;
	layout_impl.get_strides(strides);

	const auto offset = layout_impl.get_offset();

	create_or_add_operand(
		m_implementation,
		std::move(extents),
		std::move(strides),
		offset
	);

	return *this;
}

const joint_layout_implementation* 
joint_layout_builder::get_implementation() const noexcept
{
	return m_implementation.get();
}

joint_layout joint_layout_builder::build(
	joint_layout_build_flags flags
)
{
	if (m_implementation)
	{
		const auto enable_reordering = flags.contains(
			joint_layout_build_flag_bits::enable_reordering
		);
		const auto enable_coalescing = flags.contains(
			joint_layout_build_flag_bits::enable_coalescing
		);

		if (enable_reordering)
		{
			m_implementation->sort_axes_by_locality();
		}
		if (enable_coalescing)
		{
			m_implementation->coalesce_contiguous_axes();
		}
	}

	return joint_layout(std::move(m_implementation));
}

} // namespace rex
