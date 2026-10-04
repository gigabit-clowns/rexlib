// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_write_format.hpp>

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief The ability of the TIFF format to be written.
 *
 * A file is claimed on its extension alone, since it need not exist yet and
 * so has no signature to recognize.
 *
 * A file is created for one image or for a stack of several, none of whose
 * extents is zero. A stack of one image is not created, since a file of one
 * page reads back as an image, and neither is anything whose core rank is
 * not two, since the pages of a file are images.
 */
class tiff_write_format final
	: public image_write_format
{
public:
	tiff_write_format() noexcept = default;

	~tiff_write_format() override = default;

	std::string get_name() const override;

	backend_priority
	get_suitability(const image_probe &probe) const override;

	std::shared_ptr<image_writer> open(
		const image_probe &probe,
		const image_descriptor &descriptor,
		const image_metadata &metadata
	) const override;
};

} // namespace tiff
} // namespace em
} // namespace rexlib
