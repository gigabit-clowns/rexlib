// SPDX-License-Identifier: GPL-3.0-only

#include "device.hpp"

#include "command_queue.hpp"
#include "event.hpp"

#include <xmipp4/core/exceptions/invalid_operation_error.hpp>

namespace xmipp4
{
namespace cuda
{

device::device(int ordinal) noexcept
	: m_ordinal(ordinal)
{
}

device::~device() = default;

int device::get_ordinal() const noexcept
{
	return m_ordinal;
}

const memory_resource&
device::get_memory_resource(memory_resource_affinity /*affinity*/) const
{
	throw invalid_operation_error(
		"The CUDA backend does not expose any memory resource yet."
	);
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
