// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/device_event.hpp>
#include <xmipp4/core/hardware/device_to_host_event.hpp>

#include "../dynamic_shared_object.h"

#include <cuda_runtime.h>

namespace xmipp4 
{
namespace hardware
{

class device_queue;
class cuda_device_queue;



class cuda_event final
	: public device_event
	, public device_to_host_event
{
public:
	using handle = cudaEvent_t;

	XMIPP4_HARDWARE_CUDA_API
	cuda_event();
	cuda_event(const cuda_event &other) = delete;
	XMIPP4_HARDWARE_CUDA_API
	cuda_event(cuda_event &&other) noexcept;
	XMIPP4_HARDWARE_CUDA_API
	~cuda_event() override;

	cuda_event& operator=(const cuda_event &other) = delete;
	XMIPP4_HARDWARE_CUDA_API
	cuda_event& operator=(cuda_event &&other) noexcept;

	XMIPP4_HARDWARE_CUDA_API
	void swap(cuda_event &other) noexcept;

	XMIPP4_HARDWARE_CUDA_API
	void reset() noexcept;

	XMIPP4_HARDWARE_CUDA_API
	handle get_handle() noexcept;

	XMIPP4_HARDWARE_CUDA_API
	void signal(device_queue &queue) override;

	XMIPP4_HARDWARE_CUDA_API
	void signal(cuda_device_queue &queue);

	XMIPP4_HARDWARE_CUDA_API
	void wait() const override;

	XMIPP4_HARDWARE_CUDA_API
	void wait(device_queue &queue) const override;
	
	XMIPP4_HARDWARE_CUDA_API
	void wait(cuda_device_queue &queue) const;

	XMIPP4_HARDWARE_CUDA_API
	bool is_signaled() const override;

private:
	handle m_event;

}; 

} // namespace hardware
} // namespace xmipp4
