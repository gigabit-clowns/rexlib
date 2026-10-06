// SPDX-License-Identifier: GPL-3.0-only

#include "image_scratch_slots.hpp"

#include <rexlib/core/platform/assert.hpp>

#include <algorithm>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{

image_scratch_slots::image_scratch_slots(std::vector<std::size_t> indices)
	: m_indices(std::move(indices))
{
	const auto unordered = std::adjacent_find(
		m_indices.begin(),
		m_indices.end(),
		std::greater_equal<std::size_t>()
	);
	if (unordered != m_indices.end())
	{
		throw std::invalid_argument(
			"image_scratch_slots: The indices are not strictly ascending."
		);
	}
}

std::size_t image_scratch_slots::get_count() const noexcept
{
	return m_indices.size();
}

std::size_t image_scratch_slots::get_index(std::size_t slot) const noexcept
{
	REXLIB_ASSERT(slot < get_count());
	return m_indices[slot];
}

std::size_t
image_scratch_slots::get_lower_bound(std::size_t index) const noexcept
{
	const auto ite =
		std::lower_bound(m_indices.begin(), m_indices.end(), index);
	return static_cast<std::size_t>(std::distance(m_indices.begin(), ite));
}

std::size_t image_scratch_slots::count_held(
	std::size_t first_index,
	std::size_t index_count
) const noexcept
{
	const auto first = get_lower_bound(first_index);
	const auto max_index = std::numeric_limits<std::size_t>::max();
	if (index_count > max_index - first_index)
	{
		return get_count() - first;
	}

	return get_lower_bound(first_index + index_count) - first;
}

} // namespace em
} // namespace rexlib
