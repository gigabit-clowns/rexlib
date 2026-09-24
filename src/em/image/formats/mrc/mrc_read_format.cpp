// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_read_format.hpp"

#include "mrc_constants.hpp"
#include "mrc_extensions.hpp"
#include "mrc_header.hpp"
#include "mrc_reader.hpp"

#include <rexlib/em/image/image_probe.hpp>

#include <em/image/formats/image_format_registration_macros.hpp>

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
	return std::make_shared<mrc_reader>(
		probe.get_path(),
		get_single_section(probe.get_extension())
	);
}

REXLIB_REGISTER_IMAGE_READ_FORMAT(mrc, rexlib::em::mrc::mrc_read_format);

} // namespace mrc
} // namespace em
} // namespace rexlib
