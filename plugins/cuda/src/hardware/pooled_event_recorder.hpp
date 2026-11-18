// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "event.hpp"
#include "event_recorder.hpp"

#include <deque>
#include <mutex>
#include <vector>

namespace rexlib
{
namespace cuda
{

/**
 * @brief Captures points using recycled @ref event objects.
 *
 * Capturing a point is on the path of every command, so the events are pooled
 * and recorded again rather than created and destroyed each time.
 *
 * A pool serves one device, because an event can only be recorded on a stream
 * of the device it was created on.
 */
class pooled_event_recorder final
	: public event_recorder
{
public:
	/**
	 * @brief Construct a recorder with no events yet.
	 *
	 * @param ordinal Ordinal of the device whose streams are captured.
	 */
	explicit pooled_event_recorder(int ordinal) noexcept;
	~pooled_event_recorder() override;

	ticket record(cudaStream_t stream) override;
	void release(ticket ticket) noexcept override;
	bool is_complete(ticket ticket) override;
	void wait(ticket ticket) override;
	void enqueue_wait(cudaStream_t stream, ticket ticket) override;

private:
	int m_ordinal;
	std::mutex m_mutex;

	/// Every event ever created, in use or not. Tickets index into it, offset
	/// by one so that zero can stand for no ticket. A deque, since it leaves
	/// an event where it is when another one is created.
	std::deque<event> m_events;

	/// Tickets of the events not currently handed out.
	std::vector<ticket> m_available;

	/// The reference can be used after the lock is let go, since the event
	/// stays where it is for as long as the recorder lives.
	const event& get_event(ticket ticket);
	ticket acquire();
	ticket create();
};

} // namespace cuda
} // namespace rexlib
