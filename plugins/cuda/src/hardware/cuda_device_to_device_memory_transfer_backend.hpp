// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/memory_transfer_backend.hpp>

namespace xmipp4 
{
namespace hardware
{

class memory_transfer_manager;

class cuda_device_to_device_memory_transfer_backend final
	: public memory_transfer_backend
{
public:
	backend_priority get_suitability(
		const memory_resource& source,
		const memory_resource& destination
	) const noexcept override;

	std::shared_ptr<memory_transfer> create_transfer(
		const memory_resource& source,
		const memory_resource& destination
	) const override;

	static bool register_at(memory_transfer_manager &manager);

}; 

} // namespace hardware
} // namespace xmipp4
