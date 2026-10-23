// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/plugin_manager.hpp>
#include <rexlib/core/service_catalog.hpp>

#include <memory>
#include <string>

namespace rexlib
{

class command_queue;
class device;
class device_manager;
class memory_allocator;

namespace cuda
{

/**
 * @brief Everything needed to reach a CUDA device through the loaded plugin.
 *
 * Reaches the plugin the way an application does, through the interfaces of
 * rexlib alone. Where there is no device it holds none, and a test skips.
 */
class cuda_fixture
{
public:
	/**
	 * @brief Load the plugin and take the first device it reports.
	 */
	cuda_fixture();
	cuda_fixture(const cuda_fixture &other) = delete;
	cuda_fixture(cuda_fixture &&other) = delete;
	~cuda_fixture();

	cuda_fixture& operator=(const cuda_fixture &other) = delete;
	cuda_fixture& operator=(cuda_fixture &&other) = delete;

	/**
	 * @brief Check whether a device was found.
	 *
	 * @return true There is a device.
	 * @return false The plugin could not be loaded or reported no device.
	 */
	bool has_device() const noexcept;

	/**
	 * @brief Get the device.
	 *
	 * @return rexlib::device& The device.
	 * @pre @ref has_device must be true.
	 */
	rexlib::device& get_device() const noexcept;

	/**
	 * @brief Create an allocator on a resource of the device.
	 *
	 * @param affinity Which resource of the device.
	 * @return std::shared_ptr<memory_allocator> The allocator.
	 * @pre @ref has_device must be true.
	 */
	std::shared_ptr<memory_allocator> create_allocator(
		memory_resource_affinity affinity
	) const;

	/**
	 * @brief Create a queue the plugin did not make.
	 *
	 * @return std::shared_ptr<rexlib::command_queue> A queue of the CPU
	 * backend.
	 */
	std::shared_ptr<rexlib::command_queue> create_foreign_queue() const;

	/**
	 * @brief Get where the plugin is, relative to the tests.
	 *
	 * @return std::string The path.
	 */
	static std::string get_plugin_path();

private:
	plugin_manager m_manager;
	service_catalog m_catalog;
	std::shared_ptr<rexlib::device_manager> m_devices;
	std::shared_ptr<rexlib::device> m_device;
};

} // namespace cuda
} // namespace rexlib
