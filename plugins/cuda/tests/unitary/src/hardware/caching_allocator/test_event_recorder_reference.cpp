// SPDX-License-Identifier: GPL-3.0-only

#include "test_event_recorder_reference.hpp"

namespace xmipp4
{
namespace cuda
{

event_recorder_reference::event_recorder_reference(
	event_recorder &target
) noexcept
	: m_target(&target)
{
}

event_recorder_reference::~event_recorder_reference() = default;

event_recorder::ticket
event_recorder_reference::record(const queue_handle &queue)
{
	return m_target->record(queue);
}

void event_recorder_reference::release(ticket ticket) noexcept
{
	m_target->release(ticket);
}

bool event_recorder_reference::is_complete(ticket ticket)
{
	return m_target->is_complete(ticket);
}

void event_recorder_reference::wait(ticket ticket)
{
	m_target->wait(ticket);
}

void event_recorder_reference::enqueue_wait(
	const queue_handle &queue,
	ticket ticket
)
{
	m_target->enqueue_wait(queue, ticket);
}

} // namespace cuda
} // namespace xmipp4
