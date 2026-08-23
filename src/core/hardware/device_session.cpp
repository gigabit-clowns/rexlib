// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/hardware/device_session.hpp>

#include <xmipp4/core/hardware/device.hpp>
#include <xmipp4/core/hardware/command_queue.hpp>
#include <xmipp4/core/platform/assert.hpp>

#include "memory_allocator_table.hpp"

#include <stdexcept>
#include <utility>

namespace xmipp4
{

class device_session::implementation
{
public:
	implementation(
		std::shared_ptr<device> dev,
		device_properties properties
	)
		: m_device(std::move(dev))
		, m_properties(std::move(properties))
		, m_allocators(*m_device)
		, m_default_queue(m_device->create_command_queue())
	{
	}

	const std::shared_ptr<device>& get_device() const noexcept
	{
		REX_ASSERT(m_device);
		return m_device;
	}

	const device_properties& get_properties() const noexcept
	{
		return m_properties;
	}

	const std::shared_ptr<memory_allocator>&
	get_allocator(memory_resource_affinity affinity) const noexcept
	{
		return m_allocators.get_allocator(affinity);
	}

	const std::shared_ptr<command_queue>& get_default_queue() const noexcept
	{
		REX_ASSERT(m_default_queue);
		return m_default_queue;
	}

private:
	std::shared_ptr<device> m_device;
	device_properties m_properties;
	memory_allocator_table m_allocators;
	std::shared_ptr<command_queue> m_default_queue;
};


device_session::device_session(
	std::shared_ptr<device> dev,
	device_properties properties
)
{
	if (!dev)
	{
		throw std::invalid_argument(
			"Cannot construct a device_session from a null device."
		);
	}

	m_implementation = std::make_unique<implementation>(
		std::move(dev),
		std::move(properties)
	);
}

device_session::~device_session() = default;

const std::shared_ptr<device>& device_session::get_device() const noexcept
{
	REX_ASSERT(m_implementation);
	return m_implementation->get_device();
}

const device_properties& device_session::get_properties() const noexcept
{
	REX_ASSERT(m_implementation);
	return m_implementation->get_properties();
}

const std::shared_ptr<memory_allocator>&
device_session::get_allocator(
	memory_resource_affinity affinity
) const noexcept
{
	REX_ASSERT(m_implementation);
	return m_implementation->get_allocator(affinity);
}

const std::shared_ptr<command_queue>&
device_session::get_default_queue() const noexcept
{
	REX_ASSERT(m_implementation);
	return m_implementation->get_default_queue();
}

} // namespace xmipp4
