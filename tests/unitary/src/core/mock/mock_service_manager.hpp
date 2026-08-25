// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/service_manager.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_service_manager final
	: public service_manager
{
public:
	MAKE_MOCK0(register_builtin_backends, void(), override);
};

} // namespace rexlib
