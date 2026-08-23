// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/layout/broadcast.hpp>

#include <xmipp4/core/layout/broadcast_error.hpp>
#include <xmipp4/core/layout/strided_layout.hpp>
#include <xmipp4/core/platform/assert.hpp>

#include <stdexcept>
#include <sstream>

namespace xmipp4 
{

namespace
{

bool broadcast_extent(std::size_t &extent1, std::size_t &extent2)
{
	if (extent1 == extent2)
	{
		return true;
	}

	if (extent1 == 1)
	{
		extent1 = extent2;
	}
	else if (extent2 == 1)
	{
		extent2 = extent1;
	}
	else
	{
		return false;
	}

	return true;
}

void pad_extents(
	std::vector<std::size_t> &extents, 
	std::size_t target_size
)
{
	REX_ASSERT( extents.size() < target_size );

	const auto padding = target_size - extents.size();
	extents.insert(extents.cbegin(), padding, 1U);
}

bool is_axis_broadcast_compatible(
	std::size_t extent1, 
	std::size_t extent2
) noexcept
{
	return extent1 == extent2 || extent1 == 1 || extent2 == 1;
}

bool is_axis_broadcastable_to(
	std::size_t from_extent, 
	std::size_t to_extent
) noexcept
{
	return from_extent == to_extent || from_extent == 1;
}

} // anonymous namespace


void broadcast_extents(
	std::vector<std::size_t> &extents1, 
	std::vector<std::size_t> &extents2 
)
{
	if (extents1.size() < extents2.size())
	{
		pad_extents(extents1, extents2.size());
	}
	
	if (extents2.size() < extents1.size())
	{
		pad_extents(extents2, extents1.size());
	}

	REX_ASSERT( extents1.size() == extents2.size() );
	const auto n = extents1.size();
	for (std::size_t i = 0; i < n; ++i)
	{
		if (!broadcast_extent(extents1[i], extents2[i]))
		{
			throw broadcast_error(extents1, extents2);
		}
	}
}

void broadcast_extents_accumulate(
	std::vector<std::size_t> &result, 
	span<const std::size_t> other
)
{
	if (result.size() < other.size())
	{
		pad_extents(result, other.size());
	}

	// When other is shorter it is implicitly padded with leading ones, which
	// leaves the corresponding leading axes of result untouched. Therefore the
	// two sets of extents are aligned on their trailing axes.
	REX_ASSERT( result.size() >= other.size() );
	const auto padding = result.size() - other.size();
	const auto n = other.size();
	for (std::size_t i = 0; i < n; ++i)
	{
		auto tmp = other[i]; // Allow write
		if (!broadcast_extent(result[padding+i], tmp))
		{
			throw broadcast_error(
				result,
				std::vector<std::size_t>(other.begin(), other.end())
			);
		}
	}
}

bool is_broadcast_compatible(
	span<const std::size_t> extents1,
	span<const std::size_t> extents2
) noexcept
{
	if (extents1.size() < extents2.size())
	{
		std::swap(extents1, extents2);
	}

	REX_ASSERT( extents1.size() >= extents2.size() );
	const auto padding = extents1.size() - extents2.size();
	const auto n = extents2.size();
	REX_ASSERT( padding + n == extents1.size() );

	for (std::size_t i = 0; i < n; ++i)
	{
		if (!is_axis_broadcast_compatible(extents1[padding+i], extents2[i]))
		{
			return false;
		}
	}

	return true;
}

bool is_broadcastable_to(
	span<const std::size_t> from_extents,
	span<const std::size_t> to_extents
) noexcept
{
	if (from_extents.size() > to_extents.size())
	{
		return false;
	}

	REX_ASSERT( to_extents.size() >= from_extents.size() );
	const auto padding = to_extents.size() - from_extents.size();
	const auto n = from_extents.size();
	REX_ASSERT( padding + n == to_extents.size() );

	for (std::size_t i = 0; i < n; ++i)
	{
		if (!is_axis_broadcastable_to(from_extents[i], to_extents[padding+i]))
		{
			return false;
		}
	}

	return true;
}

} // namespace xmipp4
