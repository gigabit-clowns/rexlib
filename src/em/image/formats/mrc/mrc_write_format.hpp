// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_write_format.hpp>

namespace rexlib
{
namespace em
{
namespace mrc
{

/**
 * @brief The ability of the MRC format to be written.
 *
 * A file is claimed on its extension alone, since it need not exist yet and
 * so has no header to recognize.
 *
 * A stack of one image can only be created with the extension `.mrcs`, and
 * a single image only with another one, so that the file reads back as it
 * was declared.
 */
class mrc_write_format final
	: public image_write_format
{
public:
	mrc_write_format() noexcept = default;

	~mrc_write_format() override = default;

	std::string get_name() const override;

	backend_priority
	get_suitability(const image_probe &probe) const override;

	std::shared_ptr<image_writer> open(
		const image_probe &probe,
		const image_descriptor &descriptor,
		const image_metadata &metadata
	) const override;
};

} // namespace mrc
} // namespace em
} // namespace rexlib
