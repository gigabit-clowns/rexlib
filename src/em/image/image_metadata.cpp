// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/image_metadata.hpp>

#include <utility>

namespace rexlib
{
namespace em
{

image_metadata::image_metadata() noexcept = default;

image_metadata::image_metadata(std::vector<double> sampling)
	: m_sampling(std::move(sampling))
{
}

image_metadata::image_metadata(const image_metadata &other) = default;
image_metadata::image_metadata(image_metadata &&other) noexcept = default;
image_metadata::~image_metadata() = default;

image_metadata&
image_metadata::operator=(const image_metadata &other) = default;
image_metadata&
image_metadata::operator=(image_metadata &&other) noexcept = default;

span<const double> image_metadata::get_sampling() const noexcept
{
	return make_span(m_sampling.data(), m_sampling.size());
}

} // namespace em
} // namespace rexlib
