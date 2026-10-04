// SPDX-License-Identifier: GPL-3.0-only

#include "image_file_layout.hpp"

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include <functional>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace em
{

image_file_layout::image_file_layout(
	image_descriptor descriptor,
	std::vector<std::ptrdiff_t> strides,
	std::size_t data_offset,
	byte_order order
)
	: m_descriptor(std::move(descriptor))
	, m_strides(std::move(strides))
	, m_data_offset(data_offset)
	, m_byte_order(order)
{
	if (m_strides.size() != m_descriptor.get_extents().size())
	{
		throw std::invalid_argument(
			"image_file_layout: The strides do not have the rank of the "
			"extents."
		);
	}

	const auto element_size = get_size(m_descriptor.get_data_type());
	if (element_size == 0 || m_data_offset % element_size != 0)
	{
		throw image_format_error(
			"image_file_layout: The values of the file do not begin at an "
			"offset their data type can be addressed at."
		);
	}
}

const image_descriptor& image_file_layout::get_descriptor() const noexcept
{
	return m_descriptor;
}

span<const std::ptrdiff_t> image_file_layout::get_strides() const noexcept
{
	return make_span(m_strides.data(), m_strides.size());
}

std::size_t image_file_layout::get_data_offset() const noexcept
{
	return m_data_offset;
}

std::size_t image_file_layout::get_data_size() const noexcept
{
	const auto extents = m_descriptor.get_extents();
	const auto element_count = std::accumulate(
		extents.begin(),
		extents.end(),
		std::size_t(1),
		std::multiplies<std::size_t>()
	);

	return element_count * get_size(m_descriptor.get_data_type());
}

byte_order image_file_layout::get_byte_order() const noexcept
{
	return m_byte_order;
}

} // namespace em
} // namespace rexlib
