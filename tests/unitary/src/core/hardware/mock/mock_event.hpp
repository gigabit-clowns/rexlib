// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/hardware/event.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_event final
	: public event
{
public:
	MAKE_CONST_MOCK0(
		get_supported_usage,
		event_usage_flags(),
		noexcept override
	);
	MAKE_CONST_MOCK0(wait, void(), override);
	MAKE_CONST_MOCK0(is_signaled, bool(), override);
};

} // namespace rexlib
