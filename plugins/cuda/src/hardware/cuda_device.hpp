// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/device.hpp>

#include <memory>

namespace rexlib
{
namespace cuda
{

class device_memory_resource;
class event_recorder;

/**
 * @brief CUDA implementation of @ref rexlib::device.
 *
 * The handle is a thin wrapper around a device ordinal, so several handles
 * may refer to the same physical device without interfering with each other.
 * The one driver resource it holds is the pool of events its queues capture
 * their commands with, which fills as they are needed.
 */
class cuda_device final
	: public device
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
	explicit cuda_device(int ordinal);
	cuda_device(const cuda_device &other) = delete;
	cuda_device(cuda_device &&other) = delete;
	~cuda_device() override;

	cuda_device& operator=(const cuda_device &other) = delete;
	cuda_device& operator=(cuda_device &&other) = delete;

	int get_ordinal() const noexcept;

	const memory_resource&
	get_memory_resource(memory_resource_affinity affinity) const override;

	std::shared_ptr<command_queue> create_command_queue() const override;

private:
	int m_ordinal;

	/// Shared with the queues, whose timelines may outlive this.
	std::shared_ptr<event_recorder> m_events;

	/// Null where the device shares the host's memory, since there is then no
	/// such thing as memory local to it.
	std::unique_ptr<device_memory_resource> m_memory;
};

} // namespace cuda
} // namespace rexlib
