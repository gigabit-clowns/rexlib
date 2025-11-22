// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_memory_transfer.hpp"

#include <xmipp4/cuda/hardware/cuda_device_queue.hpp>
#include <xmipp4/cuda/hardware/cuda_buffer.hpp>
#include <xmipp4/core/hardware/buffer.hpp>
#include <xmipp4/core/hardware/copy_region.hpp>
#include <xmipp4/core/memory/align.hpp>
#include <xmipp4/core/logger.hpp>

#include <stdexcept>

#include <cuda_runtime.h>

namespace xmipp4
{
namespace hardware
{

static
void cuda_copy_bytes(
	const void *src_ptr, 
	std::size_t src_size, 
	void* dst_ptr, 
	std::size_t dst_size,
	const copy_region &region,
	cudaStream_t stream
)
{
	const auto src_offset = region.get_source_offset();
	const auto dst_offset = region.get_destination_offset();
	const auto byte_count = region.get_count();

	if (src_offset + byte_count > src_size)
	{
		throw std::out_of_range(
			"Copy region exceeds source buffer size."
		);
	}

	if (dst_offset + byte_count > dst_size)
	{
		throw std::out_of_range(
			"Copy region exceeds destination buffer size."
		);
	}

	cudaMemcpyAsync(
		memory::offset_bytes(dst_ptr, dst_offset), 
		memory::offset_bytes(src_ptr, src_offset), 
		byte_count,
		cudaMemcpyDefault,
		stream
	);
}

void cuda_memory_transfer::copy(
	const buffer &source, 
	buffer &destination,
	span<const copy_region> regions, 
	device_queue *queue
) const
{
	cudaStream_t stream_handle = nullptr;
	auto *cuda_queue = dynamic_cast<cuda_device_queue*>(queue);
	if (cuda_queue)
	{
		stream_handle = cuda_queue->get_handle();
	}
	else if (queue)
	{
		XMIPP4_LOG_WARN(
			"Provided device_queue is not a cuda_device_queue. "
			"Falling back to synchronous copy."
		);
		queue->wait_until_completed(); 
	}

	const auto *src_ptr = get_source_pointer(source);
	auto *dst_ptr = get_destination_pointer(destination);
	const auto src_size = source.get_size();
	const auto dst_size = destination.get_size();
	for (const auto &region : regions)
	{
		cuda_copy_bytes(
			src_ptr,
			src_size, 
			dst_ptr,
			dst_size, 
			region, 
			stream_handle
		);
	}
}

} // namespace hardware
} // namespace xmipp4
