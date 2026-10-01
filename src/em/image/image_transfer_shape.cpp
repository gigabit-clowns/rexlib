// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_transfer_shape.hpp>

#include <rexlib/core/platform/assert.hpp>

#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{

image_transfer_shape::image_transfer_shape(
	std::vector<std::size_t> extents,
	std::size_t file_rank,
	std::size_t array_rank
)
	: m_extents(std::move(extents))
	, m_file_rank(file_rank)
	, m_array_rank(array_rank)
{
	if (m_extents.size() > m_file_rank)
	{
		throw std::invalid_argument(
			"image_transfer_shape: The extents do not fit in the rank of the "
			"file."
		);
	}

	if (m_extents.size() > m_array_rank)
	{
		throw std::invalid_argument(
			"image_transfer_shape: The extents do not fit in the rank of the "
			"array."
		);
	}
}

image_transfer_shape::image_transfer_shape(
	const image_transfer_shape &other
) = default;
image_transfer_shape::image_transfer_shape(
	image_transfer_shape &&other
) noexcept = default;
image_transfer_shape::~image_transfer_shape() = default;

image_transfer_shape&
image_transfer_shape::operator=(const image_transfer_shape &other) = default;
image_transfer_shape& image_transfer_shape::operator=(
	image_transfer_shape &&other
) noexcept = default;

span<const std::size_t> image_transfer_shape::get_extents() const noexcept
{
	return make_span(m_extents.data(), m_extents.size());
}

std::size_t image_transfer_shape::get_rank() const noexcept
{
	return m_extents.size();
}

std::size_t image_transfer_shape::get_file_rank() const noexcept
{
	return m_file_rank;
}

std::size_t image_transfer_shape::get_array_rank() const noexcept
{
	return m_array_rank;
}

std::size_t
image_transfer_shape::get_leading_rank(std::size_t rank) const noexcept
{
	REXLIB_ASSERT(get_rank() <= rank);
	return rank - get_rank();
}

std::size_t image_transfer_shape::get_extent(
	std::size_t rank,
	std::size_t axis
) const noexcept
{
	REXLIB_ASSERT(axis < rank);

	const auto leading = get_leading_rank(rank);
	return axis < leading ? 1 : m_extents[axis - leading];
}

} // namespace em
} // namespace rexlib
