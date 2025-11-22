// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/memory_transfer.hpp>

namespace xmipp4 
{
namespace hardware
{

class cuda_device_queue;
class cuda_buffer;

class cuda_memory_transfer
	: public memory_transfer
{
public:
	cuda_memory_transfer() = default;
	~cuda_memory_transfer() override = default;

	void copy(
		const buffer &source, 
		buffer &destination,
		span<const copy_region> regions, 
		device_queue *queue
	) const override;

	virtual const void* get_source_pointer(const buffer &source) const = 0;
	virtual void* get_destination_pointer(buffer &destination) const = 0; 

};

} // namespace hardware
} // namespace xmipp4
