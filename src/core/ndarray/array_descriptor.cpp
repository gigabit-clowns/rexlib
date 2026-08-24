// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/ndarray/array_descriptor.hpp>

#include <boost/functional/hash.hpp>

namespace rexlib 
{

array_descriptor::array_descriptor() noexcept
	: m_data_type(numerical_type::unknown)
{
}

array_descriptor::array_descriptor(
	strided_layout layout,
	numerical_type data_type
) noexcept
	: m_layout(std::move(layout))
	, m_data_type(data_type)
{
}

array_descriptor::array_descriptor(const array_descriptor &other) = default;
array_descriptor::array_descriptor(array_descriptor &&other) noexcept = default;
array_descriptor::~array_descriptor() = default;

array_descriptor&
array_descriptor::operator=(const array_descriptor &other) = default;
array_descriptor&
array_descriptor::operator=(array_descriptor &&other) noexcept = default;

std::size_t array_descriptor::hash() const noexcept
{
	auto seed = m_layout.hash();
	boost::hash_combine(seed, boost::hash_value(m_data_type));
	return seed;
}

const strided_layout& array_descriptor::get_layout() const noexcept
{
	return m_layout;
}

numerical_type array_descriptor::get_data_type() const noexcept
{
	return m_data_type;
}

array_descriptor make_contiguous_array_descriptor(
	span<const std::size_t> extents,
	numerical_type data_type
)
{
	return array_descriptor(
		strided_layout::make_contiguous_layout(extents),
		data_type
	);
}

bool is_initialized(const array_descriptor &descriptor) noexcept
{
	return descriptor.get_data_type() != numerical_type::unknown;
}

std::size_t compute_storage_requirement(const array_descriptor &descriptor)
{
	const auto &layout = descriptor.get_layout();
	const auto data_type = descriptor.get_data_type();
	return layout.compute_storage_requirement() * get_size(data_type);
}

} // namespace rexlib
