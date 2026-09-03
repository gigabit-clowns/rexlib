// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_writer_provider.hpp>

#include <rexlib/em/image/image_writer.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

class mock_image_writer_provider final
	: public image_writer_provider
{
public:
	mock_image_writer_provider() = default;

	MAKE_MOCK1(
		acquire,
		std::shared_ptr<image_writer>(const std::string &path),
		override
	);

	MAKE_MOCK0(flush, void(), override);
};

} // namespace em
} // namespace rexlib
