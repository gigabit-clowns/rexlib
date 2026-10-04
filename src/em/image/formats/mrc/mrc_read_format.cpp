// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_read_format.hpp"

#include "mrc_constants.hpp"
#include "mrc_extensions.hpp"
#include "mrc_geometry.hpp"
#include "mrc_header.hpp"

#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>

#include <em/image/formats/image_format_registration_macros.hpp>
#include <em/image/formats/memory_mapping/mapped_image_reader.hpp>

namespace rexlib
{
namespace em
{
namespace mrc
{

std::string mrc_read_format::get_name() const
{
	return "MRC";
}

backend_priority
mrc_read_format::get_suitability(const image_probe &probe) const
{
	if (has_map_identifier(probe.get_leading_bytes()))
	{
		return backend_priority::normal;
	}

	if (is_readable_extension(probe.get_extension()) &&
		probe.get_leading_bytes().size() >= header_size)
	{
		return backend_priority::fallback;
	}

	return backend_priority::unsupported;
}

std::shared_ptr<image_reader>
mrc_read_format::open(const image_probe &probe) const
{
	const auto header = parse_header(probe.get_leading_bytes());

	return std::make_shared<mapped_image_reader>(
		probe.get_path(),
		derive_file_layout(
			header,
			get_single_section(probe.get_extension())
		),
		image_metadata()
	);
}

REXLIB_REGISTER_IMAGE_READ_FORMAT(mrc, rexlib::em::mrc::mrc_read_format);

} // namespace mrc
} // namespace em
} // namespace rexlib
