// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/plugin.hpp>
#include <rexlib/core/service_catalog.hpp>
#include <rexlib/core/version.hpp>

#include <trompeloeil.hpp>

#include <string>

namespace rexlib
{

class mock_plugin final
	: public plugin
{
public:
	MAKE_CONST_MOCK0(get_name, const std::string&(), noexcept override);
	MAKE_CONST_MOCK0(get_version, version(), noexcept override);
	MAKE_CONST_MOCK1(register_at, void(service_catalog&), override);
};

} // namespace rexlib
