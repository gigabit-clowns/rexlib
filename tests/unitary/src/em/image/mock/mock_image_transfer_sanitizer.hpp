// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_transfer_sanitizer.hpp>

#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <trompeloeil.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

class mock_image_transfer_sanitizer final
	: public image_transfer_sanitizer
{
public:
	mock_image_transfer_sanitizer() = default;

	MAKE_CONST_MOCK3(
		sanitize,
		std::vector<image_transfer_plan>(
			const image_transfer_plan &regions,
			span<const std::size_t> file_extents,
			span<const std::size_t> array_extents
		),
		override
	);
};

} // namespace em
} // namespace rexlib
