// SPDX-License-Identifier: GPL-3.0-only

#include "pooled_event_recorder.hpp"

#include "error.hpp"

#include <cstddef>

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

pooled_event_recorder::pooled_event_recorder(int ordinal) noexcept
	: m_ordinal(ordinal)
{
}

pooled_event_recorder::~pooled_event_recorder() = default;

event_recorder::ticket pooled_event_recorder::record(cudaStream_t stream)
{
	// Nobody else holds the ticket yet, so the event can be recorded without
	// the lock.
	const auto result = acquire();
	try
	{
		REXLIB_CUDA_CHECK(
			cudaEventRecord(get_event(result).get_handle(), stream)
		);
	}
	catch (...)
	{
		release(result);
		throw;
	}

	return result;
}

void pooled_event_recorder::release(ticket ticket) noexcept
{
	if (ticket == no_ticket)
	{
		return;
	}

	// Room was reserved when the event was created, so this cannot throw.
	const std::lock_guard<std::mutex> lock(m_mutex);
	m_available.push_back(ticket);
}

bool pooled_event_recorder::is_complete(ticket ticket)
{
	return get_event(ticket).is_signaled();
}

void pooled_event_recorder::wait(ticket ticket)
{
	get_event(ticket).wait();
}

void pooled_event_recorder::enqueue_wait(cudaStream_t stream, ticket ticket)
{
	REXLIB_CUDA_CHECK(
		cudaStreamWaitEvent(
			stream,
			get_event(ticket).get_handle(),
			cudaEventWaitDefault
		)
	);
}

const event& pooled_event_recorder::get_event(ticket ticket)
{
	const std::lock_guard<std::mutex> lock(m_mutex);
	return m_events[ticket - 1];
}

event_recorder::ticket pooled_event_recorder::acquire()
{
	const std::lock_guard<std::mutex> lock(m_mutex);

	if (m_available.empty())
	{
		return create();
	}

	const auto result = m_available.back();
	m_available.pop_back();
	return result;
}

event_recorder::ticket pooled_event_recorder::create()
{
	// Giving a ticket back has to succeed, since it happens where a failure
	// can no longer be handled. Reserving its room before the event exists is
	// what makes that allocation free.
	m_available.reserve(m_events.size() + 1);

	m_events.emplace_back(m_ordinal);
	return m_events.size();
}

} // namespace cuda
} // namespace rexlib
