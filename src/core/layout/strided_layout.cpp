// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/layout/strided_layout.hpp>

#include "strided_layout_implementation.hpp"

namespace rexlib 
{

strided_layout::strided_layout() noexcept = default;
strided_layout::strided_layout(const strided_layout &other) noexcept = default;
strided_layout::strided_layout(strided_layout &&other) noexcept = default;
strided_layout::~strided_layout() = default;

strided_layout&
strided_layout::operator=(const strided_layout &other) noexcept = default;
strided_layout&
strided_layout::operator=(strided_layout &&other) noexcept = default;

strided_layout::strided_layout(
	std::shared_ptr<const strided_layout_implementation> impl
) noexcept
	: m_implementation(std::move(impl))
{
}

strided_layout::strided_layout(strided_layout_implementation &&impl)
	: m_implementation(
		std::make_shared<strided_layout_implementation>(std::move(impl))
	)
{
}


std::size_t strided_layout::hash() const noexcept
{
	return get_implementation().hash();
}

std::size_t strided_layout::get_rank() const noexcept
{
	return get_implementation().get_rank();
}

void strided_layout::get_extents(std::vector<std::size_t> &extents) const
{
	return get_implementation().get_extents(extents);
}

void strided_layout::get_strides(std::vector<std::ptrdiff_t> &strides) const
{
	return get_implementation().get_strides(strides);
}

REXLIB_NODISCARD
std::ptrdiff_t strided_layout::get_offset() const noexcept
{
	return get_implementation().get_offset();
}

REXLIB_NODISCARD
std::size_t strided_layout::compute_storage_requirement() const noexcept
{
	return get_implementation().compute_storage_requirement();
}

REXLIB_NODISCARD
std::size_t strided_layout::compute_element_count() const noexcept
{
	return get_implementation().compute_element_count();
}

REXLIB_NODISCARD
bool strided_layout::extents_equal(
	span<const std::size_t> extents
) const noexcept
{
	return get_implementation().extents_equal(extents);
}

REXLIB_NODISCARD
strided_layout 
strided_layout::apply_subscripts(span<const dynamic_subscript> subscripts) const
{
	if (subscripts.empty())
	{
		return *this; // No change
	}

	return strided_layout(get_implementation().apply_subscripts(subscripts));
}

REXLIB_NODISCARD
strided_layout strided_layout::transpose() const
{
	if (get_rank() <= 1)
	{   
		return *this; // Empty or single axis. Not modified.
	}

	REXLIB_ASSERT( m_implementation );
	const auto &impl = *m_implementation;
	return strided_layout(impl.transpose());
}

REXLIB_NODISCARD
strided_layout strided_layout::permute(span<const std::size_t> order) const
{
	return strided_layout(get_implementation().permute(order));
}

REXLIB_NODISCARD
strided_layout 
strided_layout::matrix_transpose(
	std::ptrdiff_t axis1, 
	std::ptrdiff_t axis2) const
{
	return strided_layout(get_implementation().matrix_transpose(axis1, axis2));
}

REXLIB_NODISCARD
strided_layout 
strided_layout::matrix_diagonal(
	std::ptrdiff_t axis1, 
	std::ptrdiff_t axis2
) const
{
	return strided_layout(get_implementation().matrix_diagonal(axis1, axis2));
}

REXLIB_NODISCARD
strided_layout strided_layout::squeeze() const
{
	if (m_implementation)
	{
		const auto &impl = *m_implementation;
		return strided_layout(impl.squeeze());
	}
	else
	{
		return strided_layout();
	}
}

REXLIB_NODISCARD
strided_layout 
strided_layout::broadcast_to(span<const std::size_t> extents) const
{
	if (extents_equal(extents))
	{
		return *this; // Re-use
	}

	return strided_layout(get_implementation().broadcast_to(extents));
}

const strided_layout_implementation&
strided_layout::get_implementation() const noexcept
{
	static const strided_layout_implementation empty_implementation;
	return m_implementation ? *m_implementation : empty_implementation;
}

REXLIB_NODISCARD
strided_layout strided_layout::make_contiguous_layout(
	span<const std::size_t> extents
)
{
	if (extents.empty())
	{
		return strided_layout(); // Empty layout
	}

	strided_layout_implementation::strided_axis_vector_type axes;
	axes.reserve(extents.size());
	std::transform(
		extents.begin(), extents.end(),
		std::back_inserter(axes),
		[] (std::size_t extent)
		{
			return strided_axis(extent, 0);
		}
	);

	std::ptrdiff_t stride = 1;
	for (auto ite = axes.rbegin(); ite != axes.rend(); ++ite)
	{
		ite->set_stride(stride);
		stride *= ite->get_extent();
	}

	return strided_layout(strided_layout_implementation(axes, 0));
}

REXLIB_NODISCARD
strided_layout strided_layout::make_custom_layout(
	span<const std::size_t> extents, 
	span<const std::ptrdiff_t> strides, 
	std::ptrdiff_t offset
)
{
	if (extents.size() != strides.size())
	{
		throw std::invalid_argument(
			"Extents and strides must have the same number of elements"
		);
	}
	
	const auto count = extents.size();
	if (count == 0 && offset == 0)
	{
		return strided_layout(); // Empty layout
	}

	strided_layout_implementation::strided_axis_vector_type axes;
	axes.reserve(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		axes.emplace_back(extents[i], strides[i]);
	}

	return strided_layout(
		strided_layout_implementation(axes, offset)
	);
}

bool operator==(const strided_layout &lhs, const strided_layout &rhs) noexcept
{
	return lhs.get_implementation() == rhs.get_implementation();
}

} // namespace rexlib
