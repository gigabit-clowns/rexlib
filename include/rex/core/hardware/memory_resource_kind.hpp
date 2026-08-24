// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/platform/dynamic_shared_object.h>

namespace rexlib 
{

/**
 * @brief Describes the nature of a memory resource.
 */
enum class memory_resource_kind
{
	/// The device and host share the same physical memory, such that both can 
	/// access it efficiently.
	unified,

	/// The device and host have their own physical memory, although transfers
	/// are managed by the driver. Therefore, buffers can be treated as if they
	/// were residing in unified memory.
	managed,

	/// Memory resource lives locally on the device and can not be accessed 
	/// from the host.
	device_local,

	/// Memory resource resides in the host although it can be accessed from
	/// the device at a certain performance penalty.
	device_mapped,

	/// Memory resource resides in the host and can not be accessed from the
	/// device. However, contents can be transferred from and to the device.
	host_staging,

	/// Memory resource resides in the host and can not be accessed nor 
	/// transferred to a device.
	host,
};

/**
 * @brief Assign a numeric affinity score for host use to a memory kind.
 *
 * Higher values indicate memory that is more suitable for host access.
 * Kinds that are fully host-local (e.g. @c host) receive the highest
 * scores, while kinds that reside on the device and cannot be accessed
 * from the host (e.g. @c device_local) receive the lowest ones.
 *
 * @param kind Memory resource kind to be scored.
 * @return int Affinity score. Higher means more host-friendly. Returns
 * @c 0 if the memory kind is not host-accessible or @p kind is invalid.
 */
REXLIB_API
int score_host_affinity(memory_resource_kind kind) noexcept;

/**
 * @brief Assign a numeric affinity score for device use to a memory kind.
 *
 * Higher values indicate memory that is more suitable for device access.
 * Kinds that are fully device-local (e.g. @c device_local) receive the
 * highest scores, while kinds that reside on the host and cannot be
 * accessed from the device (e.g. @c host) receive the lowest ones.
 *
 * @param kind Memory resource kind to be scored.
 * @return int Affinity score. Higher means more device-friendly. Returns
 * @c 0 if the memory kind is not device-accessible or @p kind is invalid.
 */
REXLIB_API
int score_device_affinity(memory_resource_kind kind) noexcept;

/**
 * @brief Check if the memory kind can be directly accessed from the host.
 * 
 * @param kind Memory resource kind to be checked.
 * @return true If the memory resource can be accessed from the host.
 * @return false If the memory resource can not be accessed from the host.
 * or an invalid kind was given.
 */
REXLIB_API
bool is_host_accessible(memory_resource_kind kind) noexcept;

/**
 * @brief Check if the memory kind can be directly accessed from the device.
 * 
 * @param kind Memory resource kind to be checked.
 * @return true If the memory resource can be accessed from the device.
 * @return false If the memory resource can not be accessed from the device.
 * or an invalid kind was given.
 */
REXLIB_API
bool is_device_accessible(memory_resource_kind kind) noexcept;

} // namespace rexlib
