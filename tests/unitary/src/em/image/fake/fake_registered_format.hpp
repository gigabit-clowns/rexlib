// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "fake_image_reader.hpp"

#include <rexlib/em/image/image_read_format.hpp>

#include <string>

namespace rexlib
{
namespace em
{

/**
 * @brief Name the format registered by the registration macro reports.
 */
const char *const fake_registered_format_name = "fake";

/**
 * @brief A read format registered through REXLIB_REGISTER_IMAGE_READ_FORMAT.
 *
 * Exists so that the registration macro, the private core registry and the
 * draining done by register_builtin_backends are exercised the way a real
 * format would exercise them. It is linked into the unitary test binary
 * alone and so is never part of the library.
 */
class fake_registered_format final
	: public image_read_format
{
public:
	fake_registered_format() = default;

	std::string get_name() const override;

	backend_priority
	get_suitability(const image_probe &probe) const override;

	std::unique_ptr<image_reader> open(
		const image_probe &probe
	) const override;
};

} // namespace em
} // namespace rexlib
