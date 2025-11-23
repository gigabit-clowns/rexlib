// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/device_queue.hpp>

#include "../dynamic_shared_object.h"

#include <cuda_runtime.h>

namespace xmipp4 
{
namespace hardware
{

class cuda_device;

class cuda_device_queue final
	: public device_queue
{
public:
	using handle = cudaStream_t;

	XMIPP4_CUDA_API
	explicit cuda_device_queue(cuda_device &device);
	cuda_device_queue(const cuda_device_queue &other) = delete;
	XMIPP4_CUDA_API
	cuda_device_queue(cuda_device_queue &&other) noexcept;
	XMIPP4_CUDA_API
	~cuda_device_queue() override;

	cuda_device_queue& operator=(const cuda_device_queue &other) = delete;
	XMIPP4_CUDA_API
	cuda_device_queue& operator=(cuda_device_queue &&other) noexcept;

	XMIPP4_CUDA_API
	void swap(cuda_device_queue &other) noexcept;

	XMIPP4_CUDA_API
	void reset() noexcept;

	XMIPP4_CUDA_API
	handle get_handle() noexcept;

	XMIPP4_CUDA_API
	void wait_until_completed() const override;

	XMIPP4_CUDA_API
	bool is_idle() const noexcept override;

private:
	handle m_stream;

}; 

} // namespace hardware
} // namespace xmipp4
