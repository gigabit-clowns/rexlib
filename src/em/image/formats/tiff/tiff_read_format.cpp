// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_read_format.hpp"

#include "tiff_reader.hpp"
#include "tiff_signature.hpp"

#include <rexlib/em/image/image_probe.hpp>

#include <em/image/formats/image_format_registration_macros.hpp>

namespace rexlib
{
namespace em
{
namespace tiff
{

std::string tiff_read_format::get_name() const
{
	return "TIFF";
}

backend_priority
tiff_read_format::get_suitability(const image_probe &probe) const
{
	return has_signature(probe.get_leading_bytes())
		? backend_priority::normal
		: backend_priority::unsupported;
}

std::shared_ptr<image_reader>
tiff_read_format::open(const image_probe &probe) const
{
	return std::make_shared<tiff_reader>(probe.get_path());
}

REXLIB_REGISTER_IMAGE_READ_FORMAT(tiff, rexlib::em::tiff::tiff_read_format);

} // namespace tiff
} // namespace em
} // namespace rexlib
