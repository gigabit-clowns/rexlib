// SPDX-License-Identifier: GPL-3.0-only

#include "event_ticket.hpp"

#include <utility>

namespace xmipp4
{
namespace cuda
{

event_ticket::event_ticket() noexcept
	: m_recorder(nullptr)
	, m_ticket(event_recorder::no_ticket)
{
}

event_ticket::event_ticket(
	event_recorder &recorder,
	const queue_handle &queue
)
	: m_recorder(&recorder)
	, m_ticket(recorder.record(queue))
{
}

event_ticket::event_ticket(event_ticket &&other) noexcept
	: m_recorder(other.m_recorder)
	, m_ticket(other.m_ticket)
{
	other.m_recorder = nullptr;
	other.m_ticket = event_recorder::no_ticket;
}

event_ticket::~event_ticket()
{
	reset();
}

event_ticket& event_ticket::operator=(event_ticket &&other) noexcept
{
	event_ticket(std::move(other)).swap(*this);
	return *this;
}

void event_ticket::swap(event_ticket &other) noexcept
{
	std::swap(m_recorder, other.m_recorder);
	std::swap(m_ticket, other.m_ticket);
}

void event_ticket::reset() noexcept
{
	if (m_recorder)
	{
		m_recorder->release(m_ticket);
		m_recorder = nullptr;
		m_ticket = event_recorder::no_ticket;
	}
}

event_ticket::operator bool() const noexcept
{
	return m_recorder != nullptr;
}

bool event_ticket::is_complete() const
{
	return !m_recorder || m_recorder->is_complete(m_ticket);
}

void event_ticket::wait() const
{
	if (m_recorder)
	{
		m_recorder->wait(m_ticket);
	}
}

void event_ticket::enqueue_wait(const queue_handle &queue) const
{
	if (m_recorder)
	{
		m_recorder->enqueue_wait(queue, m_ticket);
	}
}

} // namespace cuda
} // namespace xmipp4
