// SPDX-License-Identifier: GPL-3.0-only

#include "device_memory_source.hpp"

#include "../device_guard.hpp"
#include "../error.hpp"

#include "../../config.hpp"

#include <cuda_runtime.h>

namespace xmipp4
{
namespace cuda
{

device_memory_source::device_memory_source(int ordinal) noexcept
	: m_ordinal(ordinal)
{
}

device_memory_source::~device_memory_source() = default;

int device_memory_source::get_ordinal() const noexcept
{
	return m_ordinal;
}

void* device_memory_source::allocate(std::size_t size)
{
	void *result = nullptr;

	const device_guard guard(m_ordinal);
	XMIPP4_CUDA_CHECK_ALLOCATION( cudaMalloc(&result, size) );

	return result;
}

void device_memory_source::deallocate(void *data) noexcept
{
	// Acts on the device that owns the memory, so the current one is
	// irrelevant here.
	XMIPP4_CUDA_CHECK_NO_THROW( cudaFree(data) );
}

bool device_memory_source::is_host_accessible() const noexcept
{
	return false;
}

std::size_t device_memory_source::get_base_alignment() const noexcept
{
	return XMIPP4_CUDA_ALLOCATION_ALIGN_BYTES;
}

} // namespace cuda
} // namespace xmipp4
