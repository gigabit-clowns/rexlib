// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "device_type.hpp"
#include "../platform/dynamic_shared_object.h"

#include <memory>
#include <string>

namespace rexlib
{

/**
 * @brief Description of the static characteristics of a physical device
 * visible from the host.
 *
 * device_properties is a value-type container populated by a
 * @ref device_backend (typically via
 * @ref device_backend::get_device_properties) and consumed by callers
 * that need to inspect a device without instantiating it. Instances are
 * copyable and movable and use the pimpl idiom, so the binary
 * representation is owned by the implementation and unrelated to the
 * size of this class.
 */
class device_properties
{
public:
	REXLIB_API
	device_properties() noexcept;
	REXLIB_API
	device_properties(const device_properties& other);
	REXLIB_API
	device_properties(device_properties&& other) noexcept;
	REXLIB_API
	~device_properties();

	REXLIB_API
	device_properties& operator=(const device_properties& other);
	REXLIB_API
	device_properties& operator=(device_properties&& other) noexcept;

	/**
	 * @brief Set the device type.
	 *
	 * Classifies the device into one of the broad categories defined by
	 * @ref device_type (CPU, discrete GPU, integrated GPU, etc.). Used
	 * by higher-level dispatch logic to pick reasonable defaults without
	 * knowing the specific backend.
	 *
	 * @param[in] type The device type.
	 */
	REXLIB_API
	void set_type(device_type type);

	/**
	 * @brief Get the device type.
	 *
	 * @return device_type The device type, or @ref device_type::unknown if no
	 * value has been set.
	 */
	REXLIB_API
	device_type get_type() const noexcept;

	/**
	 * @brief Set the device name.
	 *
	 * The name is meant to be a human-readable identifier suitable for
	 * display (e.g. the model reported by the vendor driver) and is not
	 * required to be unique.
	 *
	 * @param[in] name The device name.
	 */
	REXLIB_API
	void set_name(const std::string &name);

	/**
	 * @brief Get the device name.
	 *
	 * @return const std::string& The device name, or an empty string if
	 * none has been set. The reference is valid until the next
	 * non-const operation on this object.
	 */
	REXLIB_API
	const std::string& get_name() const noexcept;

	/**
	 * @brief Set the physical location of the device.
	 *
	 * Usually this is a PCI address but the format is backend defined
	 * and intended only for diagnostics and matching against external
	 * tooling.
	 *
	 * @param[in] location The physical location of the device.
	 */
	REXLIB_API
	void set_physical_location(const std::string &location);

	/**
	 * @brief Get the physical location of the device.
	 *
	 * Usually this is a PCI address; see @ref set_physical_location for
	 * the format conventions.
	 *
	 * @return const std::string& The physical location of the device,
	 * or an empty string if none has been set. The reference is valid
	 * until the next non-const operation on this object.
	 */
	REXLIB_API
	const std::string& get_physical_location() const noexcept;

	/**
	 * @brief Set the total memory of the device in bytes.
	 *
	 * This is the total amount of memory that the device exposes to the
	 * framework, not the currently free amount, and represents an upper
	 * bound for how much can ever be allocated on it.
	 *
	 * @param[in] bytes Number of bytes.
	 */
	REXLIB_API
	void set_total_memory_bytes(std::size_t bytes);

	/**
	 * @brief Get the total memory of the device in bytes.
	 *
	 * @return std::size_t Number of bytes, or zero if the value has not
	 * been set by the backend.
	 */
	REXLIB_API
	std::size_t get_total_memory_bytes() const noexcept;

	/**
	 * @brief Set the optimal data alignment.
	 *
	 * Buffers aligned to this boundary typically yield the best
	 * throughput on this device (e.g. SIMD width on CPUs, coalesced
	 * access boundary on GPUs). The value should be a power of two.
	 *
	 * @param[in] alignment The optimal data alignment in bytes.
	 */
	REXLIB_API
	void set_optimal_data_alignment(std::size_t alignment);

	/**
	 * @brief Get the optimal data alignment.
	 *
	 * @return std::size_t The optimal data alignment in bytes, or zero
	 * if no value has been set.
	 */
	REXLIB_API
	std::size_t get_optimal_data_alignment() const noexcept;

private:
	class implementation;
	std::unique_ptr<implementation> m_implementation;

	implementation& create_implementation_if_null();
	const implementation& get_implementation() const noexcept;
};

} // namespace rexlib
