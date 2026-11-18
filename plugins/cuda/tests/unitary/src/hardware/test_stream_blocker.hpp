// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <future>

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

/**
 * @brief Holds a stream busy until it is told to let go.
 *
 * Issues work on the stream that does not finish until @ref release is
 * called. Whatever is issued on the stream after it stays unfinished until
 * then, which is what lets a test look at a command that is still running
 * without racing the device.
 */
class stream_blocker
{
public:
	/**
	 * @brief Hold a stream busy.
	 *
	 * @param stream The stream.
	 */
	explicit stream_blocker(cudaStream_t stream);
	stream_blocker(const stream_blocker &other) = delete;
	stream_blocker(stream_blocker &&other) = delete;
	~stream_blocker();

	stream_blocker& operator=(const stream_blocker &other) = delete;
	stream_blocker& operator=(stream_blocker &&other) = delete;

	/**
	 * @brief Let the stream go on.
	 *
	 * Does nothing the second time.
	 */
	void release() noexcept;

private:
	std::promise<void> m_released;
	bool m_is_released;

	/// Run by the driver in the order of the stream, on a thread of its own.
	/// Owns the future it is handed, so that it does not depend on this still
	/// being around.
	static void CUDART_CB wait_until_released(void *data);
};

} // namespace cuda
} // namespace rexlib
