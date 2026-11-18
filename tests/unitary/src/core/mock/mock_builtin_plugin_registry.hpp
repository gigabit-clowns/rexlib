// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/plugin.hpp>

#include <trompeloeil.hpp>

#include <memory>

namespace rexlib
{

class mock_builtin_plugin_registry final
{
public:
	MAKE_MOCK1(add, void(std::unique_ptr<plugin> instance));
};

} // namespace rexlib
