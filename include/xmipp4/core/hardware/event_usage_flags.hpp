// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/platform/dynamic_shared_object.h>
#include <xmipp4/core/binary/bit.hpp>
#include <xmipp4/core/binary/flagset.hpp>

namespace rex 
{

/**
 * @brief Capability bits describing how an event may be used.
 *
 * Each bit represents a distinct synchronization capability that an event
 * may support. The set of bits requested at event creation lets the backend
 * pick the cheapest underlying primitive that satisfies the requested
 * capabilities.
 */
enum class event_usage_flag_bits
{
	/// The event can be queried from the host thread.
	host_query = bit(0),

	/// The event can be waited on from the host thread.
	host_wait = bit(1),

	/// The event can be waited on another queue from the same device that
	/// signals it.
	device_wait = bit(2),

	/// The event can be waited on from a queue on a different device.
	cross_device_wait = bit(3)
};

/**
 * @brief Set of @ref event_usage_flag_bits.
 */
using event_usage_flags = flagset<event_usage_flag_bits>;

} // namespace rex
