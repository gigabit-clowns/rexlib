// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/command_queue.hpp>

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

/**
 * @brief CUDA implementation of @ref rexlib::command_queue, backed by a
 * cudaStream_t.
 */
class command_queue final
	: public rexlib::command_queue
{
public:
	using handle = cudaStream_t;

	explicit command_queue(int ordinal);
	command_queue(const command_queue &other) = delete;
	command_queue(command_queue &&other) = delete;
	~command_queue() override;

	command_queue& operator=(const command_queue &other) = delete;
	command_queue& operator=(command_queue &&other) = delete;

	handle get_handle() const noexcept;
	int get_ordinal() const noexcept;

	/**
	 * @brief Block the calling thread until the stream is idle.
	 *
	 * @throws error If the stream cannot be synchronized.
	 */
	void synchronize() const;

	void submit(const command &cmd) override;
	void signal(rexlib::event &ev) override;
	void wait(const rexlib::event &ev) override;

	/**
	 * @brief Downcast a queue to this backend's implementation.
	 *
	 * @param queue The queue to be cast.
	 * @return command_queue& The same queue, as a CUDA queue.
	 *
	 * @throws std::invalid_argument If the queue was not created by this
	 * backend.
	 */
	static command_queue& cast(rexlib::command_queue &queue);

	/**
	 * @brief Downcast a queue to this backend's implementation.
	 *
	 * @param queue The queue to be cast.
	 * @return const command_queue& The same queue, as a CUDA queue.
	 *
	 * @throws std::invalid_argument If the queue was not created by this
	 * backend.
	 */
	static const command_queue& cast(const rexlib::command_queue &queue);

private:
	handle m_stream;
	int m_ordinal;
};

} // namespace cuda
} // namespace rexlib
