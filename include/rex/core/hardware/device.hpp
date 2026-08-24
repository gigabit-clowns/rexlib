// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>

#include <rex/core/platform/dynamic_shared_object.h>

#include "memory_resource_affinity.hpp"
#include "event_usage_flags.hpp"

namespace rexlib 
{

class command_queue;
class event;
class memory_resource;

/**
 * @brief Abstract representation of a hardware compute device.
 *
 * A @c device is the entry point for interacting with a backend's compute
 * resource: it advertises the memory resources reachable from the device and
 * acts as a factory for the per-device objects used by the rest of the
 * framework (queues, synchronization primitives, ...).
 */
class REXLIB_API device
{
public:
	device() noexcept;
	device(const device &other) = delete;
	device(device &&other) = delete;
	virtual ~device();

	device& operator=(const device &other) = delete;
	device& operator=(device &&other) = delete;

	/**
	 * @brief Retrieve a memory resource by its affinity.
	 *
	 * Returns the @c memory_resource that is best suited for the access pattern
	 * described by @p affinity. The returned reference is valid for the
	 * lifetime of this @c device.
	 *
	 * @note Depending on the backend, two distinct affinity values may resolve
	 * to the same underlying @c memory_resource (i.e. the references may
	 * alias).
	 *
	 * @param affinity The intended access pattern for which a resource is
	 * requested.
	 * @return A reference to the matching @c memory_resource.
	 *
	 * @see memory_resource_affinity
	 * @see memory_resource
	 */
	virtual
	const memory_resource&
	get_memory_resource(memory_resource_affinity affinity) const = 0;

	/**
	 * @brief Create a new command queue on this device.
	 *
	 * The returned queue is independent from any previously created queue on 
	 * the same device, including the default queue and may execute its commands
	 * concurrently with them. Ordering across queues may be established through
	 * @ref event objects.
	 *
	 * @return Newly created queue. Never null.
	 */
	virtual std::shared_ptr<command_queue> create_command_queue() const = 0;

	/**
	 * @brief Create a synchronization primitive for this device.
	 *
	 * The returned event supports at least the operations requested in
	 * @p usage; the backend may pick the cheapest underlying primitive that 
	 * satisfies them, so the actually supported set (queried via
	 * @ref event::get_supported_usage) may be a superset of @p usage. 
	 * The event is initially in the signaled state (waiting on it returns 
	 * immediately).
	 *
	 * @param usage Capabilities that the returned event must support.
	 * @return Newly created event. Never null.
	 *
	 * @see event
	 * @see event_usage_flags
	 */
	virtual
	std::shared_ptr<event> create_event(event_usage_flags usage) const = 0;
};

} // namespace rexlib
