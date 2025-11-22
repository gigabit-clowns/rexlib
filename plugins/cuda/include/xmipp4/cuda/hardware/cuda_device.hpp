// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/device.hpp>
#include <xmipp4/core/hardware/memory_resource.hpp>

#include "../dynamic_shared_object.h"

#include <memory>

namespace xmipp4 
{
namespace hardware
{

class cuda_device final
	: public device
{
public:
	XMIPP4_HARDWARE_CUDA_API
	explicit cuda_device(int device_index);
	cuda_device(const cuda_device &other) = delete;
	cuda_device(cuda_device &&other) = delete;
	~cuda_device() override = default;

	cuda_device& operator=(const cuda_device &other) = delete;
	cuda_device& operator=(cuda_device &&other) = delete;

	XMIPP4_HARDWARE_CUDA_API
	int get_index() const noexcept;

	XMIPP4_HARDWARE_CUDA_API
	void enumerate_memory_resources(
		std::vector<memory_resource*> &resources
	) override;
	
	XMIPP4_HARDWARE_CUDA_API
	std::shared_ptr<device_queue>
	create_device_queue() override;

	XMIPP4_HARDWARE_CUDA_API
	std::shared_ptr<device_event> create_device_event() override;

	XMIPP4_HARDWARE_CUDA_API
	std::shared_ptr<device_to_host_event> 
	create_device_to_host_event() override;

private:
	int m_device_index;
	std::unique_ptr<memory_resource> m_device_local_memory_resource;

}; 

} // namespace hardware
} // namespace xmipp4
