// SPDX-License-Identifier: GPL-3.0-only

#include "stream_timeline.hpp"

#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace cuda
{

stream_timeline::stream_timeline(std::shared_ptr<event_recorder> recorder)
	: m_recorder(std::move(recorder))
	, m_first_id(0)
	, m_waiter_count(0)
{
	if (!m_recorder)
	{
		throw std::invalid_argument(
			"stream_timeline::stream_timeline: An event recorder is required."
		);
	}
}

stream_timeline::~stream_timeline()
{
	for (const auto point : m_points)
	{
		m_recorder->release(point);
	}
}

void stream_timeline::wait(std::size_t id) const
{
	event_recorder::ticket point;
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		if (is_known_complete(id))
		{
			return;
		}

		point = get_point(id);
		++m_waiter_count;
	}

	// Waited for without the lock, so that a host wait holds up neither the
	// commands being submitted to the stream nor whoever asks about them.
	try
	{
		m_recorder->wait(point);
	}
	catch (...)
	{
		end_wait();
		throw;
	}

	end_wait();
}

bool stream_timeline::is_complete(std::size_t id) const
{
	const std::lock_guard<std::mutex> lock(m_mutex);
	return is_known_complete(id) || m_recorder->is_complete(get_point(id));
}

std::size_t stream_timeline::record(cudaStream_t stream)
{
	const std::lock_guard<std::mutex> lock(m_mutex);

	if (m_waiter_count == 0)
	{
		drop_reached_points();
	}

	const auto point = m_recorder->record(stream);
	try
	{
		m_points.push_back(point);
	}
	catch (...)
	{
		m_recorder->release(point);
		throw;
	}

	return m_first_id + m_points.size() - 1;
}

void stream_timeline::enqueue_wait(cudaStream_t waiter, std::size_t id) const
{
	const std::lock_guard<std::mutex> lock(m_mutex);

	if (!is_known_complete(id))
	{
		m_recorder->enqueue_wait(waiter, get_point(id));
	}
}

bool stream_timeline::is_known_complete(std::size_t id) const noexcept
{
	return id < m_first_id;
}

event_recorder::ticket stream_timeline::get_point(std::size_t id) const
{
	const auto index = id - m_first_id;
	if (index >= m_points.size())
	{
		throw std::out_of_range(
			"stream_timeline: The timeline never gave that id to a command."
		);
	}

	return m_points[index];
}

void stream_timeline::drop_reached_points()
{
	// The stream runs in order, so the first point that has not been reached
	// is where the ones that have end.
	while (!m_points.empty() && m_recorder->is_complete(m_points.front()))
	{
		m_recorder->release(m_points.front());
		m_points.pop_front();
		++m_first_id;
	}
}

void stream_timeline::end_wait() const noexcept
{
	const std::lock_guard<std::mutex> lock(m_mutex);
	--m_waiter_count;
}

} // namespace cuda
} // namespace rexlib
