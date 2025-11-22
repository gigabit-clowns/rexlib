// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/memory_resource.hpp>

#include <xmipp4/cuda/hardware/cuda_device.hpp>

#include <utility>

namespace xmipp4 
{
namespace hardware
{

class cuda_device;

class cuda_device_memory_resource final
	: public memory_resource
{
public:
	explicit cuda_device_memory_resource(cuda_device &device) noexcept;
	cuda_device_memory_resource(const cuda_device_memory_resource &other) = default;
	cuda_device_memory_resource(cuda_device_memory_resource &&other) = default;
	~cuda_device_memory_resource() override = default;

	cuda_device_memory_resource&
	operator=(const cuda_device_memory_resource &other) = default;
	cuda_device_memory_resource&
	operator=(cuda_device_memory_resource &&other) = default;

	cuda_device* get_target_device() const noexcept override;

	memory_resource_kind get_kind() const noexcept override;

	std::size_t get_max_heap_alignment() const noexcept override;

	std::shared_ptr<memory_heap> 
	create_memory_heap(std::size_t size, std::size_t alignment) override;

private:
	std::reference_wrapper<cuda_device> m_device;

}; 

} // namespace hardware
} // namespace xmipp4

