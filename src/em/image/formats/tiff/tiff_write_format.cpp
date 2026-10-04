// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_write_format.hpp"

#include "tiff_extensions.hpp"
#include "tiff_sample_type.hpp"
#include "tiff_writer.hpp"

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_probe.hpp>

#include <em/image/formats/image_format_registration_macros.hpp>

#include <algorithm>
#include <cstddef>

namespace rexlib
{
namespace em
{
namespace tiff
{

namespace
{

const std::size_t page_rank = 2;
const std::size_t stack_rank = 3;

} // anonymous namespace

std::string tiff_write_format::get_name() const
{
	return "TIFF";
}

backend_priority
tiff_write_format::get_suitability(const image_probe &probe) const
{
	return is_writable_extension(probe.get_extension())
		? backend_priority::normal
		: backend_priority::unsupported;
}

std::shared_ptr<image_writer> tiff_write_format::open(
	const image_probe &probe,
	const image_descriptor &descriptor,
	const image_metadata &/*metadata*/
) const
{
	const auto extents = descriptor.get_extents();
	const auto rank = extents.size();
	if ((rank != page_rank && rank != stack_rank) ||
		descriptor.get_core_rank() != page_rank)
	{
		throw unsupported_operation_error(
			probe.get_path() + ": tiff_write_format: A TIFF file holds an "
			"image or a stack of images."
		);
	}

	// A file of one page reads back as an image, so a stack of one would
	// read back as another shape than it is created with.
	if (rank == stack_rank && extents[0] == 1)
	{
		throw unsupported_operation_error(
			probe.get_path() + ": tiff_write_format: A stack of one image "
			"would read back as a single image."
		);
	}

	if (std::find(extents.begin(), extents.end(), 0) != extents.end())
	{
		throw unsupported_operation_error(
			probe.get_path() + ": tiff_write_format: A TIFF file has no "
			"extent of zero."
		);
	}

	if (!is_supported(descriptor.get_data_type()))
	{
		throw unsupported_operation_error(
			probe.get_path() + ": tiff_write_format: The TIFF format has no "
			"sample this format transfers for this data type."
		);
	}

	return std::make_shared<tiff_writer>(probe.get_path(), descriptor);
}

REXLIB_REGISTER_IMAGE_WRITE_FORMAT(tiff, rexlib::em::tiff::tiff_write_format);

} // namespace tiff
} // namespace em
} // namespace rexlib
