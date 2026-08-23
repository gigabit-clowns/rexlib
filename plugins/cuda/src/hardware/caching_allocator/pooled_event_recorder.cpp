// SPDX-License-Identifier: GPL-3.0-only

#include "pooled_event_recorder.hpp"

#include "queue_handle.hpp"

#include "../error.hpp"
#include "../event.hpp"

#include <cstddef>
#include <stdexcept>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace cuda
{

pooled_event_recorder::pooled_event_recorder() noexcept = default;

pooled_event_recorder::~pooled_event_recorder() = default;

event_recorder::ticket
pooled_event_recorder::record(const queue_handle &queue)
{
	if (!queue)
	{
		throw std::invalid_argument(
			"pooled_event_recorder::record: A queue is required to capture a "
			"point of its timeline."
		);
	}

	const auto result = acquire(queue.get_ordinal());
	try
	{
		XMIPP4_CUDA_CHECK(
			cudaEventRecord(get_event(result).get_handle(), queue.get_stream())
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
	const auto ordinal = get_event(ticket).get_ordinal();
	m_available[static_cast<std::size_t>(ordinal)].push_back(ticket);
}

bool pooled_event_recorder::is_complete(ticket ticket)
{
	return get_event(ticket).is_signaled();
}

void pooled_event_recorder::wait(ticket ticket)
{
	get_event(ticket).wait();
}

void pooled_event_recorder::enqueue_wait(
	const queue_handle &queue,
	ticket ticket
)
{
	if (!queue)
	{
		throw std::invalid_argument(
			"pooled_event_recorder::enqueue_wait: A queue is required to defer "
			"its work."
		);
	}

	XMIPP4_CUDA_CHECK(
		cudaStreamWaitEvent(
			queue.get_stream(),
			get_event(ticket).get_handle(),
			cudaEventWaitDefault
		)
	);
}

event& pooled_event_recorder::get_event(ticket ticket) const noexcept
{
	return *m_events[ticket - 1];
}

std::vector<event_recorder::ticket>&
pooled_event_recorder::get_available(int ordinal)
{
	const auto index = static_cast<std::size_t>(ordinal);
	if (index >= m_available.size())
	{
		m_available.resize(index + 1);
	}

	return m_available[index];
}

event_recorder::ticket pooled_event_recorder::acquire(int ordinal)
{
	auto &available = get_available(ordinal);
	if (available.empty())
	{
		return create(ordinal);
	}

	const auto result = available.back();
	available.pop_back();
	return result;
}

event_recorder::ticket pooled_event_recorder::create(int ordinal)
{
	// Every bit of room is made before the event exists, so that nothing can
	// throw once it does and leave it stranded. Giving a ticket back has to
	// succeed in particular, since it happens where a failure can no longer be
	// handled, and reserving here is what makes it allocation free.
	const auto count = m_events.size() + 1;
	get_available(ordinal).reserve(count);
	if (count > m_events.capacity())
	{
		m_events.reserve(2 * count);
	}

	m_events.push_back(std::make_unique<event>(ordinal));
	return m_events.size();
}

} // namespace cuda
} // namespace xmipp4
