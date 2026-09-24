// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_write_format.hpp"

#include "mrc_extensions.hpp"
#include "mrc_writer.hpp"

#include <rexlib/em/image/image_probe.hpp>

#include <em/image/formats/image_format_registration.hpp>

namespace rexlib
{
namespace em
{
namespace mrc
{

std::string mrc_write_format::get_name() const
{
	return "MRC";
}

backend_priority
mrc_write_format::get_suitability(const image_probe &probe) const
{
	return is_writable_extension(probe.get_extension())
		? backend_priority::normal
		: backend_priority::unsupported;
}

std::shared_ptr<image_writer> mrc_write_format::open(
	const image_probe &probe,
	span<const std::size_t> extents,
	std::size_t core_rank,
	numerical_type data_type,
	const image_metadata &/*metadata*/
) const
{
	// Nothing of the metadata reaches the file: image_metadata states
	// nothing yet.
	return std::make_shared<mrc_writer>(
		probe.get_path(),
		extents,
		core_rank,
		data_type
	);
}

REXLIB_REGISTER_IMAGE_WRITE_FORMAT(mrc, rexlib::em::mrc::mrc_write_format);

} // namespace mrc
} // namespace em
} // namespace rexlib
