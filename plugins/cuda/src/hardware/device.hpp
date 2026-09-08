// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/device.hpp>

#include <memory>

namespace rexlib
{
namespace cuda
{

class device_memory_resource;

/**
 * @brief CUDA implementation of @ref rexlib::device.
 *
 * The handle is a thin wrapper around a device ordinal: it owns no driver
 * resource of its own, so several handles may refer to the same physical
 * device without interfering with each other.
 */
class device final
	: public rexlib::device
{
public:
	/**
	 * @brief Construct a handle to a device.
	 *
	 * @param ordinal Ordinal of the device.
	 *
	 * @throws error If the device could not be asked what kind of memory it
	 * has.
	 */
	explicit device(int ordinal);
	device(const device &other) = delete;
	device(device &&other) = delete;
	~device() override;

	device& operator=(const device &other) = delete;
	device& operator=(device &&other) = delete;

	int get_ordinal() const noexcept;

	const memory_resource&
	get_memory_resource(memory_resource_affinity affinity) const override;

	std::shared_ptr<rexlib::command_queue> create_command_queue() const override;

	std::shared_ptr<rexlib::event>
	create_event(event_usage_flags usage) const override;

private:
	int m_ordinal;

	/// Null where the device shares the host's memory, since there is then no
	/// such thing as memory local to it.
	std::unique_ptr<device_memory_resource> m_memory;
};

} // namespace cuda
} // namespace rexlib
