// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "device_index.hpp"
#include "device_properties.hpp"
#include "device_backend.hpp"
#include "../service_manager.hpp"
#include "../platform/dynamic_shared_object.h"

#include <string>

#include <memory>
#include <vector>

namespace rexlib
{

class device;
class device_session;

/**
 * @brief Centralize multiple @ref device_backend sessions and route
 * device queries to the appropriate one.
 *
 * The manager owns the registered backends and exposes a unified view of
 * all devices known to the framework, identified across backends by
 * @ref device_index. Backends are typically contributed by plugins at
 * load time via @ref register_backend, with @ref register_builtin_backends
 * adding those that are statically linked.
 */
class REXLIB_API device_manager final
	: public service_manager
{
public:
	device_manager();
	device_manager(const device_manager &other) = delete;
	device_manager(device_manager &&other) noexcept = delete;
	~device_manager() override;

	device_manager& operator=(const device_manager &other) = delete;
	device_manager& operator=(device_manager &&other) = delete;

	void register_builtin_backends() override;

	/**
	 * @brief Enumerate the names of all registered backends.
	 *
	 * @param[out] names Output parameter with the names of the contained
	 * backends. Cleared before being populated.
	 */
	void enumerate_backends(std::vector<std::string> &names) const;

	/**
	 * @brief Get a backend contained in this manager by its name.
	 *
	 * @param name The name of the backend.
	 * @return device_backend* The retrieved backend. nullptr if none is
	 * found.
	 */
	device_backend* get_backend(const std::string &name) const;

	/**
	 * @brief Register a new backend.
	 *
	 * Takes ownership of @p backend and keeps it alive for the rest of
	 * the manager's lifetime, indexed by the name it reports through
	 * @ref device_backend::get_name. Registration fails if another backend
	 * with the same name is already registered or if @p backend is null;
	 * on failure, ownership of @p backend remains with the caller (the
	 * unique_ptr is not consumed) and the manager state is unchanged.
	 *
	 * @param[in] backend The backend to be registered. Must be non-null
	 * for registration to succeed.
	 * @return true The backend was successfully registered and is now
	 * owned by the manager.
	 * @return false A name collision occurred or @p backend was null;
	 * the manager is left unchanged.
	 */
	bool register_backend(std::unique_ptr<device_backend> backend);

	/**
	 * @brief Enumerate devices across all registered backends.
	 *
	 * Asks every registered backend to enumerate its devices and
	 * aggregates the results into @p indices. The relative order of
	 * backends in the output is implementation defined; within a single
	 * backend the order matches whatever
	 * @ref device_backend::enumerate_devices reports.
	 *
	 * @param[out] indices Output list of device indices. Cleared before
	 * being populated, so existing contents are discarded.
	 */
	void enumerate_devices(std::vector<device_index> &indices) const;

	/**
	 * @brief Query the properties of a device.
	 *
	 * Looks up the backend named by @p index and forwards the query to
	 * @ref device_backend::get_device_properties. Fails if the backend
	 * is not registered or if the backend itself does not recognize the
	 * device id.
	 *
	 * @param[in] index Index identifying the target device.
	 * @param[out] properties Output device properties. Only modified
	 * when the call succeeds.
	 * @return true The device exists and @p properties has been written.
	 * @return false The backend or device referenced by @p index does
	 * not exist; @p properties is left untouched.
	 */
	bool get_device_properties(
		const device_index &index,
		device_properties &properties
	) const;

	/**
	 * @brief Create a device session.
	 *
	 * Resolves the backend named by @p index, delegates the actual device
	 * instantiation to @ref device_backend::create_device and queries its
	 * @ref device_properties, then bundles both into a @ref device_session
	 * (which also selects the default allocators for the device).
	 *
	 * Instances are not cached: each call returns a freshly created
	 * @ref device_session.
	 *
	 * @param[in] index Index identifying the target device.
	 * @return std::shared_ptr<device_session> A non-null session.
	 * @throws std::invalid_argument If no backend is registered under the
	 * name carried by @p index, or if the backend does not recognize the
	 * device id. The backend may additionally throw if the device id is
	 * invalid.
	 */
	std::shared_ptr<device_session>
	create_device_session(const device_index &index) const;

private:
	class implementation;
	REXLIB_STD_MEMBER_INTERFACE 
	std::unique_ptr<implementation> m_implementation;

	void create_implementation_if_null();
}; 

} // namespace rexlib
