// SPDX-License-Identifier: GPL-3.0-only

#include "device.hpp"

#include "command_queue.hpp"
#include "event.hpp"

#include "device_memory_resource.hpp"
#include "pinned_memory_resource.hpp"

#include <stdexcept>

namespace xmipp4
{
namespace cuda
{

device::device(int ordinal)
	: m_ordinal(ordinal)
{
	// Where the host and the device are the same physical memory, a device
	// local pool would be a second name for the page locked one plus a copy
	// nothing needs, so both kinds of request are served from the latter.
	if (pinned_memory_resource::get().get_kind() !=
	    memory_resource_kind::unified)
	{
		m_memory = std::make_unique<device_memory_resource>(ordinal);
	}
}

device::~device() = default;

int device::get_ordinal() const noexcept
{
	return m_ordinal;
}

const memory_resource&
device::get_memory_resource(memory_resource_affinity affinity) const
{
	if (!m_memory)
	{
		return pinned_memory_resource::get();
	}

	switch (affinity)
	{
	case memory_resource_affinity::device:
		return *m_memory;

	case memory_resource_affinity::host:
		// Page locked, so that transfers to and from the device can run
		// asynchronously and at full speed.
		return pinned_memory_resource::get();

	default:
		throw std::invalid_argument(
			"device::get_memory_resource: Unknown memory resource affinity."
		);
	}
}

std::shared_ptr<xmipp4::command_queue> device::create_command_queue() const
{
	return std::make_shared<command_queue>(m_ordinal);
}

std::shared_ptr<xmipp4::event>
device::create_event(event_usage_flags /*usage*/) const
{
	return std::make_shared<event>(m_ordinal);
}

} // namespace cuda
} // namespace xmipp4
