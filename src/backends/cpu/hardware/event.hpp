// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/hardware/event.hpp>

namespace rex
{
namespace cpu
{

/**
 * @brief CPU implementation of @ref event.
 *
 * Supports @ref event_usage_flag_bits::host_query,
 * @ref event_usage_flag_bits::host_wait and
 * @ref event_usage_flag_bits::device_wait.
 */
class event
	: public rex::event
{
public:
	event_usage_flags get_supported_usage() const noexcept override;

	void wait() const override;
	bool is_signaled() const override;
};

} // namespace cpu
} // namespace rex
