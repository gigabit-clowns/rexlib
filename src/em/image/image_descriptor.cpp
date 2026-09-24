// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_descriptor.hpp>

#include <stdexcept>
#include <utility>

#include <boost/functional/hash.hpp>

namespace rexlib
{
namespace em
{

image_descriptor::image_descriptor(
	span<const std::size_t> extents,
	std::size_t core_rank,
	numerical_type data_type
)
	: m_extents(extents.begin(), extents.end())
	, m_core_rank(core_rank)
	, m_data_type(data_type)
{
	if (core_rank == 0)
	{
		throw std::invalid_argument(
			"image_descriptor: The core rank must not be zero."
		);
	}

	if (core_rank > extents.size())
	{
		throw std::invalid_argument(
			"image_descriptor: The core rank must not exceed the rank of the "
			"extents."
		);
	}

	if (data_type == numerical_type::unknown)
	{
		throw std::invalid_argument(
			"image_descriptor: The data type must be known."
		);
	}
}

image_descriptor::image_descriptor(const image_descriptor &other) = default;
image_descriptor::image_descriptor(
	image_descriptor &&other
) noexcept = default;
image_descriptor::~image_descriptor() = default;

image_descriptor&
image_descriptor::operator=(const image_descriptor &other) = default;
image_descriptor&
image_descriptor::operator=(image_descriptor &&other) noexcept = default;

std::size_t image_descriptor::hash() const noexcept
{
	auto seed = boost::hash_range(m_extents.cbegin(), m_extents.cend());
	boost::hash_combine(seed, boost::hash_value(m_core_rank));
	boost::hash_combine(seed, boost::hash_value(m_data_type));
	return seed;
}

span<const std::size_t> image_descriptor::get_extents() const noexcept
{
	return make_span(m_extents.data(), m_extents.size());
}

std::size_t image_descriptor::get_core_rank() const noexcept
{
	return m_core_rank;
}

numerical_type image_descriptor::get_data_type() const noexcept
{
	return m_data_type;
}

span<const std::size_t>
get_core_extents(const image_descriptor &descriptor) noexcept
{
	const auto extents = descriptor.get_extents();
	const auto core_rank = descriptor.get_core_rank();
	return make_span(
		extents.data() + (extents.size() - core_rank),
		core_rank
	);
}

} // namespace em
} // namespace rexlib
