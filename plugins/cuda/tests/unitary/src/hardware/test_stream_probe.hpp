// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <atomic>
#include <memory>

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

/**
 * @brief Work for a stream that says whether it has finished.
 *
 * The work takes a moment, so that a stream that was not waited for is still
 * busy with it when a test looks.
 */
class stream_probe
{
public:
	stream_probe();
	stream_probe(const stream_probe &other) = delete;
	stream_probe(stream_probe &&other) = delete;
	~stream_probe();

	stream_probe& operator=(const stream_probe &other) = delete;
	stream_probe& operator=(stream_probe &&other) = delete;

	/**
	 * @brief Issue the work on a stream.
	 *
	 * @param stream The stream.
	 */
	void issue(cudaStream_t stream);

	/**
	 * @brief Check whether the work has finished.
	 *
	 * @return true Every piece of work issued so far has finished, and at
	 * least one was issued.
	 * @return false Some of it is still to run, or none was issued.
	 */
	bool has_finished() const noexcept;

private:
	std::shared_ptr<std::atomic<int>> m_finished_count;
	int m_issued_count;

	/// Run by the driver in the order of the stream, on a thread of its own.
	/// Owns the counter it is handed, so that it does not depend on this
	/// still being around.
	static void CUDART_CB finish(void *data);
};

} // namespace cuda
} // namespace rexlib
