// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rex 
{

/**
 * @brief Enumeration to query memory_resource according to its intended usage.
 */
enum class memory_resource_affinity
{
	/// The memory resource can be optimally accessed by the host.
	host, 
	/// The memory resource can be optimally accessed by the device.
	device,

	// Add here.

	count
};

} // namespace rex
