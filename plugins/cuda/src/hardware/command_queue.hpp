// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/command_queue.hpp>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace cuda
{

/**
 * @brief CUDA implementation of @ref xmipp4::command_queue, backed by a
 * cudaStream_t.
 */
class command_queue final
	: public xmipp4::command_queue
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
	void signal(xmipp4::event &ev) override;
	void wait(const xmipp4::event &ev) override;

private:
	handle m_stream;
	int m_ordinal;
};

} // namespace cuda
} // namespace xmipp4
