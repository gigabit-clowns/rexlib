// SPDX-License-Identifier: GPL-3.0-only

#include "image_prefetch_policy.hpp"

#include <rexlib/core/system/host.hpp>

#include <algorithm>

namespace rexlib
{
namespace em
{

image_prefetch_policy::image_prefetch_policy(
	std::size_t gap_tolerance,
	std::size_t byte_budget,
	std::size_t page_size
) noexcept
	: m_gap_tolerance(gap_tolerance)
	, m_byte_budget(byte_budget)
	, m_page_size(page_size)
{
}

std::size_t image_prefetch_policy::get_gap_tolerance() const noexcept
{
	return m_gap_tolerance;
}

std::size_t image_prefetch_policy::get_byte_budget() const noexcept
{
	return m_byte_budget;
}

std::size_t image_prefetch_policy::get_page_size() const noexcept
{
	return m_page_size;
}

image_prefetch_policy make_prefetch_policy(std::size_t region_span) noexcept
{
	const auto page_size = get_page_size();
	const auto floor = std::max(region_span, page_size);

	return image_prefetch_policy(
		std::min(floor, default_prefetch_gap_cap),
		default_prefetch_budget,
		page_size
	);
}

} // namespace em
} // namespace rexlib
