// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/memory_heap.hpp>

#include <utility>

namespace xmipp4 
{
namespace hardware
{

class cuda_device_memory_resource;

class cuda_device_memory_heap final
	: public memory_heap
{
public:
	cuda_device_memory_heap(
		cuda_device_memory_resource &resource,
		std::size_t size
	);
	~cuda_device_memory_heap() override;

	std::size_t get_size() const noexcept override;
	
	std::shared_ptr<buffer> create_buffer(
		std::size_t offset, 
		std::size_t size,
		std::unique_ptr<buffer_sentinel> sentinel
	) override;

private:
	std::reference_wrapper<cuda_device_memory_resource> m_resource;
	void *m_data;
	std::size_t m_size;

}; 

} // namespace hardware
} // namespace xmipp4

