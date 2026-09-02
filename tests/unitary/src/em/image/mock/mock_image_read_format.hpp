// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_read_format.hpp>

#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_reader.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

class mock_image_read_format final
	: public image_read_format
{
public:
	mock_image_read_format() = default;

	MAKE_CONST_MOCK0(get_name, std::string(), override);

	MAKE_CONST_MOCK1(
		get_suitability,
		backend_priority(const image_probe &probe),
		override
	);

	MAKE_CONST_MOCK1(
		open,
		std::unique_ptr<image_reader>(const image_probe &probe),
		override
	);
};

} // namespace em
} // namespace rexlib
