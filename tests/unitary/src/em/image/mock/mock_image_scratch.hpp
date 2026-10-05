// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_scratch.hpp>

#include <rexlib/em/image/image_scratch_entry.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

class mock_image_scratch final
	: public image_scratch
{
public:
	mock_image_scratch() = default;

	MAKE_MOCK1(
		find,
		std::shared_ptr<image_scratch_entry>(const std::string &path),
		override
	);

	MAKE_CONST_MOCK1(
		find,
		std::shared_ptr<const image_scratch_entry>(const std::string &path),
		override
	);
};

} // namespace em
} // namespace rexlib
