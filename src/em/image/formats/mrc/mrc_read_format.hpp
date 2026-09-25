// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_read_format.hpp>

namespace rexlib
{
namespace em
{
namespace mrc
{

/**
 * @brief The ability of the MRC format to be read.
 *
 * A file is claimed on the identifier its header carries, which the probe
 * always reaches: it sits 208 bytes in and a probe holds far more than that.
 * A file that only matches by extension is claimed at
 * @ref backend_priority::fallback, which covers the files written before the
 * identifier was specified without letting this format take a file another
 * one recognizes properly.
 *
 * A file holding a single section in the image space group opens as a stack
 * of one image when its extension is `.mrcs`, and as one image otherwise.
 */
class mrc_read_format final
	: public image_read_format
{
public:
	mrc_read_format() noexcept = default;

	~mrc_read_format() override = default;

	std::string get_name() const override;

	backend_priority
	get_suitability(const image_probe &probe) const override;

	std::shared_ptr<image_reader> open(
		const image_probe &probe
	) const override;
};

} // namespace mrc
} // namespace em
} // namespace rexlib
