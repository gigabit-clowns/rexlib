// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_device.hpp"

#include "cuda_command_queue.hpp"

#include "device_memory_resource.hpp"
#include "pinned_memory_resource.hpp"
#include "pooled_event_recorder.hpp"

#include <stdexcept>

namespace rexlib
{
namespace cuda
{

cuda_device::cuda_device(int ordinal)
	: m_ordinal(ordinal)
	, m_events(std::make_shared<pooled_event_recorder>(ordinal))
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

cuda_device::~cuda_device() = default;

int cuda_device::get_ordinal() const noexcept
{
	return m_ordinal;
}

const memory_resource&
cuda_device::get_memory_resource(memory_resource_affinity affinity) const
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
			"cuda_device::get_memory_resource: Unknown memory resource "
			"affinity."
		);
	}
}

std::shared_ptr<command_queue> cuda_device::create_command_queue() const
{
	return std::make_shared<cuda_command_queue>(m_ordinal, m_events);
}

} // namespace cuda
} // namespace rexlib
