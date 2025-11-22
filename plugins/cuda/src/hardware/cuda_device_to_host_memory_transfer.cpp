// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_device_to_host_memory_transfer.hpp"

#include <xmipp4/cuda/hardware/cuda_device_queue.hpp>
#include <xmipp4/cuda/hardware/cuda_buffer.hpp>
#include <xmipp4/core/hardware/buffer.hpp>
#include <xmipp4/core/hardware/copy_region.hpp>
#include <xmipp4/core/memory/align.hpp>
#include <xmipp4/core/logger.hpp>

#include <stdexcept>

namespace xmipp4
{
namespace hardware
{

const void* cuda_device_to_host_memory_transfer::get_source_pointer(
	const buffer &source
) const
{
	const auto *ptr = cuda_get_device_ptr(source);
	if (!ptr)
	{
		throw std::invalid_argument(
			"Source buffer is not device accessible."
		);
	}

	return ptr;
}

void* cuda_device_to_host_memory_transfer::get_destination_pointer(
	buffer &destination
) const
{
	auto *ptr = destination.get_host_ptr();
	if (!ptr)
	{
		throw std::invalid_argument(
			"Destination buffer is not host accessible."
		);
	}

	return ptr;
} 

} // namespace hardware
} // namespace xmipp4
