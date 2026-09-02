// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_metadata.hpp>

#include <utility>

namespace rexlib
{
namespace em
{

image_metadata::image_metadata() noexcept = default;

image_metadata::image_metadata(
	std::vector<double> sampling,
	std::vector<double> origin
)
	: m_sampling(std::move(sampling))
	, m_origin(std::move(origin))
{
}

image_metadata::image_metadata(const image_metadata &other) = default;
image_metadata::image_metadata(image_metadata &&other) noexcept = default;
image_metadata::~image_metadata() = default;

image_metadata&
image_metadata::operator=(const image_metadata &other) = default;
image_metadata&
image_metadata::operator=(image_metadata &&other) noexcept = default;

void image_metadata::get_sampling(std::vector<double> &sampling) const
{
	sampling = m_sampling;
}

void image_metadata::get_origin(std::vector<double> &origin) const
{
	origin = m_origin;
}

} // namespace em
} // namespace rexlib
