// SPDX-License-Identifier: GPL-3.0-only

#include "event.hpp"

namespace rex
{
namespace cpu
{

event_usage_flags event::get_supported_usage() const noexcept
{
	return {
		event_usage_flag_bits::host_query,
		event_usage_flag_bits::host_wait,
		event_usage_flag_bits::device_wait
	};
}

void event::wait() const
{
	// No-op
}

bool event::is_signaled() const
{
	return true;
}

} // namespace cpu
} // namespace rex
