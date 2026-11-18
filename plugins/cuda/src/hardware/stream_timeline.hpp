// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "event_recorder.hpp"

#include <rexlib/core/hardware/command_timeline.hpp>

#include <cstddef>
#include <deque>
#include <memory>
#include <mutex>

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

/**
 * @brief Gives ids to the commands of a stream and tells when they finish.
 *
 * A timeline belongs to one stream. Ids grow by one with every command, and
 * a stream finishes its work in the order it was issued, so a command has
 * finished once any later one has.
 *
 * A point of the stream is captured for every command, and given back once
 * the command is known to have finished. The timeline therefore holds as
 * many points as it has commands that may still be running.
 *
 * It does not need the stream once the commands are issued, and stays usable
 * after the stream is destroyed.
 *
 * An id it never gave is refused with @c std::out_of_range.
 *
 * Thread safe.
 */
class stream_timeline final
	: public rexlib::command_timeline
{
public:
	/**
	 * @brief Construct a timeline with no commands.
	 *
	 * @param recorder Captures the points of the stream. Can not be nullptr.
	 *
	 * @throws std::invalid_argument If @p recorder is nullptr.
	 */
	explicit stream_timeline(std::shared_ptr<event_recorder> recorder);
	~stream_timeline() override;

	void wait(std::size_t id) const override;
	bool is_complete(std::size_t id) const override;

	/**
	 * @brief Give an id to the work issued on the stream so far.
	 *
	 * Also gives back the points of the commands that have finished, unless
	 * a thread is waiting for one of them.
	 *
	 * @param stream The stream of this timeline. Every call must name the
	 * same one.
	 * @return std::size_t The id. Greater than every id given before.
	 *
	 * @throws error If the point of the stream could not be captured, or an
	 * earlier one could not be queried.
	 */
	std::size_t record(cudaStream_t stream);

	/**
	 * @brief Defer further work on a stream until a command has finished.
	 *
	 * Does not block the calling thread, and does nothing if the command is
	 * already known to have finished.
	 *
	 * @param waiter The stream whose further work is deferred. It may belong
	 * to another device.
	 * @param id The id this timeline gave to the command.
	 *
	 * @throws std::out_of_range If this timeline never gave @p id.
	 * @throws error If the wait could not be scheduled.
	 */
	void enqueue_wait(cudaStream_t waiter, std::size_t id) const;

private:
	std::shared_ptr<event_recorder> m_recorder;
	mutable std::mutex m_mutex;

	/// One point per command that may still be running, oldest first.
	std::deque<event_recorder::ticket> m_points;

	/// Id of the command the oldest point stands for. Every command before
	/// it has finished.
	std::size_t m_first_id;

	/// How many threads are waiting for a point, which keeps every point
	/// from being given back and captured again under them.
	mutable std::size_t m_waiter_count;

	bool is_known_complete(std::size_t id) const noexcept;
	event_recorder::ticket get_point(std::size_t id) const;
	void drop_reached_points();
	void end_wait() const noexcept;
};

} // namespace cuda
} // namespace rexlib
