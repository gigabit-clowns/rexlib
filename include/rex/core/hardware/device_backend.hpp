// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "device_properties.hpp"
#include "../version.hpp"
#include "../platform/dynamic_shared_object.h"

#include <memory>
#include <string>
#include <vector>

namespace rexlib
{

class device;

/**
 * @brief Abstract class that represents a hardware backend.
 *
 * A device_backend is the integration point through which a family of
 * physical or virtual compute devices (e.g. the host CPU, a vendor's GPUs)
 * is exposed to the rest of the framework. Concrete backends are typically
 * shipped as plugins and registered into a @ref device_manager, which
 * dispatches queries to the appropriate backend based on its name.
 *
 * The interface intentionally remains small: enumerating the devices that
 * the backend can see at runtime, exposing their static characteristics
 * via @ref device_properties, and instantiating a usable @ref device
 * handle on demand. Backends are non-copyable and non-movable; they are
 * meant to live behind a stable pointer owned by the @ref device_manager.
 */
class REXLIB_API device_backend
{
public:
	device_backend() noexcept;
	device_backend(const device_backend &other) = delete;
	device_backend(device_backend &&other) = delete;
	virtual ~device_backend();

	device_backend& operator=(const device_backend &other) = delete;
	device_backend& operator=(device_backend &&other) = delete;

	/**
	 * @brief Get the name of the backend.
	 *
	 * @return std::string The name.
	 */
	virtual std::string get_name() const = 0;

	/**
	 * @brief Get the version of the backend.
	 *
	 * @return version The version.
	 */
	virtual version get_version() const = 0;

	/**
	 * @brief Enumerate available devices for this backend.
	 *
	 * Populates @p ids with an identifier for every device that this
	 * backend can currently make available. The list is expected to be
	 * cleared by the implementation before being filled.
	 *
	 * @param[out] ids Output parameter receiving the device identifiers.
	 * @note The IDs are opaque to the caller and only guaranteed to be
	 * unique within this backend; combine them with the backend name (via
	 * @ref device_index) to obtain a globally unique identifier.
	 * @note The set of IDs and their ordering are implementation defined
	 * and may change across runs (e.g. if hardware is hot-plugged or the
	 * driver reorders devices), so callers should treat the IDs as
	 * transient tokens valid for the lifetime of the backend instance.
	 */
	virtual void enumerate_devices(std::vector<std::size_t> &ids) const = 0;

	/**
	 * @brief Query the static properties of a device.
	 *
	 * Fills @p desc with descriptive information about the device that is
	 * not expected to change while the backend is alive (name, kind, total
	 * memory, etc.). This method does not allocate or initialize any
	 * device-side resources; use @ref create_device for that.
	 *
	 * @param[in] id Identifier of the device, as returned by
	 * @ref enumerate_devices.
	 * @param[out] desc Output properties. Only modified when the call
	 * succeeds.
	 * @return true The device ID exists and @p desc has been populated.
	 * @return false The device ID is not known to this backend; @p desc
	 * is left untouched.
	 */
	virtual bool get_device_properties(
		std::size_t id,
		device_properties &desc
	) const = 0;

	/**
	 * @brief Create a device handle for the given device identifier.
	 *
	 * Instantiates a ready-to-use @ref device object backed by the
	 * physical device referred to by @p id. Each call returns a fresh
	 * handle; backends are free to share underlying driver state between
	 * handles but must guarantee that the returned object can be used
	 * independently from any previous one.
	 *
	 * @param[in] id The identifier of the device, as returned by
	 * @ref enumerate_devices.
	 * @return std::shared_ptr<device> A non-null handle to the device.
	 * Backends are expected to throw if @p id is not valid rather than
	 * returning a null pointer.
	 */
	virtual std::shared_ptr<device> create_device(std::size_t id) const = 0;
};

} // namespace rexlib
