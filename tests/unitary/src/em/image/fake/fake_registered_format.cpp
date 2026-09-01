// SPDX-License-Identifier: GPL-3.0-only

#include "fake_registered_format.hpp"

#include <rexlib/em/image/image_open_options.hpp>
#include <rexlib/em/image/image_probe.hpp>

#include <em/image/formats/image_format_registration.hpp>

#include <vector>

namespace rexlib
{
namespace em
{

std::string fake_registered_format::get_name() const
{
	return fake_registered_format_name;
}

backend_priority
fake_registered_format::get_suitability(const image_probe &probe) const
{
	return probe.get_extension() == ".fake"
		? backend_priority::normal
		: backend_priority::unsupported;
}

std::unique_ptr<image_reader> fake_registered_format::open(
	const image_probe &,
	const image_open_options &
) const
{
	const std::vector<std::size_t> extents = {2, 2};
	const std::vector<std::size_t> granularity = {1, 1};
	return std::unique_ptr<image_reader>(new fake_image_reader(
		make_span(extents),
		image_access_traits(
			make_span(granularity),
			image_access_flags(),
			numerical_type::int16
		),
		image_metadata()
	));
}

} // namespace em
} // namespace rexlib

REXLIB_REGISTER_IMAGE_READ_FORMAT(fake, ::rexlib::em::fake_registered_format);
