// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "cuda_memory_transfer.hpp"

namespace xmipp4 
{
namespace hardware
{

class cuda_device_to_device_memory_transfer
	: public cuda_memory_transfer
{
public:
	cuda_device_to_device_memory_transfer() = default;
	~cuda_device_to_device_memory_transfer() override = default;

	const void* get_source_pointer(const buffer &source) const override;
	void* get_destination_pointer(buffer &destination) const override; 

};

} // namespace hardware
} // namespace xmipp4
