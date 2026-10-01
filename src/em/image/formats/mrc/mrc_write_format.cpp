// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_write_format.hpp"

#include "mrc_constants.hpp"
#include "mrc_extensions.hpp"
#include "mrc_geometry.hpp"
#include "mrc_header.hpp"

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/memory/byte.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_probe.hpp>

#include <em/image/formats/image_format_registration_macros.hpp>
#include <em/image/formats/memory_mapping/mapped_image_writer.hpp>

#include <vector>

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
	const image_descriptor &descriptor,
	const image_metadata &/*metadata*/
) const
{
	// Nothing of the metadata reaches the file: image_metadata states
	// nothing yet.
	const auto header = make_header(descriptor);
	const auto layout = derive_file_layout(
		header,
		get_single_section(probe.get_extension())
	);

	// A file that would read back as another shape than it is created with
	// would misplace every region written to it.
	if (layout.get_descriptor() != descriptor)
	{
		throw unsupported_operation_error(
			probe.get_path() + ": mrc_write_format: The file would read "
			"back as another shape than it is created with, as a stack of "
			"one image does from a file not named as a stack."
		);
	}

	std::vector<byte> preamble(header_size);
	serialize_header(header, make_span(preamble.data(), preamble.size()));

	return std::make_shared<mapped_image_writer>(
		probe.get_path(),
		layout,
		make_span(preamble.data(), preamble.size())
	);
}

REXLIB_REGISTER_IMAGE_WRITE_FORMAT(mrc, rexlib::em::mrc::mrc_write_format);

} // namespace mrc
} // namespace em
} // namespace rexlib
