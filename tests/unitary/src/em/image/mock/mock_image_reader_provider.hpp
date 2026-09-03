// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_reader_provider.hpp>

#include <rexlib/em/image/image_reader.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

class mock_image_reader_provider final
	: public image_reader_provider
{
public:
	mock_image_reader_provider() = default;

	MAKE_MOCK1(
		acquire,
		std::shared_ptr<const image_reader>(const std::string &path),
		override
	);
};

} // namespace em
} // namespace rexlib
