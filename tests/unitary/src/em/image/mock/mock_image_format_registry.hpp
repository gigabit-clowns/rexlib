// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>
#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

template <typename Format>
class mock_image_format_registry final
{
public:
	using format_type = Format;

	mock_image_format_registry() = default;

	MAKE_MOCK1(add, void(std::unique_ptr<Format> (*factory)()));
};

} // namespace em
} // namespace rexlib
