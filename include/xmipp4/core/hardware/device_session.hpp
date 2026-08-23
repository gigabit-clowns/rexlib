// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "device_properties.hpp"
#include "memory_resource_affinity.hpp"
#include "../platform/dynamic_shared_object.h"

#include <memory>

namespace xmipp4
{

class device;
class memory_allocator;
class command_queue;

/**
 * @brief Immutable aggregate of a device handle and its associated objects.
 *
 * A @c device_session bundles a @ref device handle with the objects needed to
 * make it usable: the static @ref device_properties describing it, a
 * @ref memory_allocator selected for each @ref memory_resource_affinity, and a
 * default @ref command_queue created from the device. All of these are
 * resolved once at construction and remain fixed for the lifetime of the
 * session.
 */
class device_session
{
public:
	/**
	 * @brief Construct an session wrapping @p dev.
	 *
	 * Stores @p dev and @p properties and builds the allocator for each
	 * @ref memory_resource_affinity by querying @p dev for the matching
	 * memory resource.
	 *
	 * @param dev Device to wrap. Must not be null.
	 * @param properties Static properties describing @p dev. Taken by value
	 * and moved into the session.
	 *
	 * @throws std::invalid_argument if @p dev is null.
	 * @throws std::runtime_error if a required @ref memory_resource_affinity
	 * has no suitable resource on @p dev.
	 */
	REXLIB_API
	device_session(
		std::shared_ptr<device> dev,
		device_properties properties
	);
	device_session(const device_session &other) = delete;
	device_session(device_session &&other) = delete;
	REXLIB_API
	~device_session();

	device_session& operator=(const device_session &other) = delete;
	device_session& operator=(device_session &&other) = delete;

	/**
	 * @brief Retrieve the wrapped device handle.
	 *
	 * @return A reference to the non-null device handle.
	 */
	REXLIB_API
	const std::shared_ptr<device>& get_device() const noexcept;

	/**
	 * @brief Retrieve the static properties of the wrapped device.
	 *
	 * @return A reference to the device's @ref device_properties.
	 */
	REXLIB_API
	const device_properties& get_properties() const noexcept;

	/**
	 * @brief Retrieve the allocator for the given affinity.
	 *
	 * The returned allocator is the one selected at construction for
	 * @p affinity and lives as long as this @c device_session.
	 *
	 * @param affinity The desired memory_resource_affinity (host or device).
	 * @return A reference to the non-null allocator for @p affinity.
	 */
	REXLIB_API
	const std::shared_ptr<memory_allocator>&
	get_allocator(memory_resource_affinity affinity) const noexcept;

	/**
	 * @brief Retrieve the default command queue of this session.
	 *
	 * @return A reference to the non-null default command queue.
	 *
	 * @see command_queue
	 */
	REXLIB_API
	const std::shared_ptr<command_queue>& get_default_queue() const noexcept;

private:
	class implementation;
	std::unique_ptr<implementation> m_implementation;
};

} // namespace xmipp4
