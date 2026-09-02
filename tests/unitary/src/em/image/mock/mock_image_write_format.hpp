// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_write_format.hpp>

#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_open_options.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_writer.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

class mock_image_write_format final
	: public image_write_format
{
public:
	mock_image_write_format() = default;

	MAKE_CONST_MOCK0(get_name, std::string(), override);

	MAKE_CONST_MOCK1(
		get_suitability,
		backend_priority(const image_probe &probe),
		override
	);

	MAKE_CONST_MOCK5(
		open,
		std::unique_ptr<image_writer>(
			const image_probe &probe,
			const image_open_options &options,
			span<const std::size_t> extents,
			numerical_type data_type,
			const image_metadata &metadata
		),
		override
	);
};

} // namespace em
} // namespace rexlib
