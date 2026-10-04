// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_read_format.hpp>

namespace rexlib
{
namespace em
{
namespace tiff
{

/**
 * @brief The ability of the TIFF format to be read.
 *
 * A file is claimed on the signature it begins with, whatever it is named:
 * every TIFF file carries it, classic or BigTIFF and in either byte order,
 * so its extension adds nothing.
 *
 * A file of one page opens as an image and a file of several as a stack of
 * them.
 */
class tiff_read_format final
	: public image_read_format
{
public:
	tiff_read_format() noexcept = default;

	~tiff_read_format() override = default;

	std::string get_name() const override;

	backend_priority
	get_suitability(const image_probe &probe) const override;

	std::shared_ptr<image_reader> open(
		const image_probe &probe
	) const override;
};

} // namespace tiff
} // namespace em
} // namespace rexlib
