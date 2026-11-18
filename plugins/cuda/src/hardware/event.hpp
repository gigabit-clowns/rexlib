// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

/**
 * @brief Owns a cudaEvent_t.
 *
 * An event marks a point of a stream once it is recorded on it. The host
 * can ask whether that point has been reached, or wait for it.
 */
class event final
{
public:
	using handle = cudaEvent_t;

	/**
	 * @brief Create an event on a device.
	 *
	 * @param ordinal Ordinal of the device. The event can only be recorded
	 * on streams of that device.
	 *
	 * @throws error If the event could not be created.
	 */
	explicit event(int ordinal);
	event(const event &other) = delete;
	event(event &&other) = delete;
	~event();

	event& operator=(const event &other) = delete;
	event& operator=(event &&other) = delete;

	handle get_handle() const noexcept;
	int get_ordinal() const noexcept;

	/**
	 * @brief Block the calling thread until the recorded point is reached.
	 *
	 * Returns immediately if the event was never recorded.
	 *
	 * @throws error If the point could not be waited for.
	 */
	void wait() const;

	/**
	 * @brief Check whether the recorded point has been reached, without
	 * blocking.
	 *
	 * @return true The point has been reached, or the event was never
	 * recorded.
	 * @return false The point has not been reached yet.
	 *
	 * @throws error If the point could not be queried.
	 */
	bool is_signaled() const;

private:
	handle m_event;
	int m_ordinal;
};

} // namespace cuda
} // namespace rexlib
