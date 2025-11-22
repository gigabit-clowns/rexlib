// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_host_to_device_memory_transfer.hpp"

#include <xmipp4/cuda/hardware/cuda_buffer.hpp>

namespace xmipp4
{
namespace hardware
{

const void* cuda_host_to_device_memory_transfer::get_source_pointer(
	const buffer &source
) const
{
	const auto *ptr = source.get_host_ptr();
	if (!ptr)
	{
		throw std::invalid_argument("Source buffer is not host accessible.");
	}

	return ptr;
}

void* cuda_host_to_device_memory_transfer::get_destination_pointer(
	buffer &destination
) const
{
	auto *ptr = cuda_get_device_ptr(destination);
	if (!ptr)
	{
		throw std::invalid_argument(
			"Destination buffer is not device accessible."
		);
	}

	return ptr;
} 

} // namespace hardware
} // namespace xmipp4
