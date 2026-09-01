// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_access_traits.hpp>

#include <utility>

namespace rexlib
{
namespace em
{

image_access_traits::image_access_traits() noexcept
	: m_flags()
	, m_preferred_data_type(numerical_type::unknown)
{
}

image_access_traits::image_access_traits(
	span<const std::size_t> granularity,
	image_access_flags flags,
	numerical_type preferred_data_type
)
	: m_granularity(granularity.begin(), granularity.end())
	, m_flags(flags)
	, m_preferred_data_type(preferred_data_type)
{
}

image_access_traits::image_access_traits(
	const image_access_traits &other
) = default;
image_access_traits::image_access_traits(
	image_access_traits &&other
) noexcept = default;
image_access_traits::~image_access_traits() = default;

image_access_traits&
image_access_traits::operator=(const image_access_traits &other) = default;
image_access_traits&
image_access_traits::operator=(image_access_traits &&other) noexcept = default;

void image_access_traits::get_granularity(
	std::vector<std::size_t> &granularity
) const
{
	granularity = m_granularity;
}

image_access_flags image_access_traits::get_flags() const noexcept
{
	return m_flags;
}

numerical_type
image_access_traits::get_preferred_data_type() const noexcept
{
	return m_preferred_data_type;
}

} // namespace em
} // namespace rexlib
