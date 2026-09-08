// SPDX-License-Identifier: GPL-3.0-only

#include "pinned_memory_source.hpp"

#include "../error.hpp"

#include "../../config.hpp"

#include <cuda_runtime.h>

namespace rexlib
{
namespace cuda
{

pinned_memory_source::pinned_memory_source(bool mapped) noexcept
	: m_mapped(mapped)
{
}

pinned_memory_source::~pinned_memory_source() = default;

bool pinned_memory_source::is_mapped() const noexcept
{
	return m_mapped;
}

void* pinned_memory_source::allocate(std::size_t size)
{
	// Portable, so that the one process-wide pool of page-locked memory serves
	// every device rather than only the one that happened to be current when
	// the heap was taken.
	unsigned int flags = cudaHostAllocPortable;
	if (m_mapped)
	{
		flags |= cudaHostAllocMapped;
	}

	void *result = nullptr;
	REXLIB_CUDA_CHECK_ALLOCATION( cudaHostAlloc(&result, size, flags) );

	return result;
}

void pinned_memory_source::deallocate(void *data) noexcept
{
	REXLIB_CUDA_CHECK_NO_THROW( cudaFreeHost(data) );
}

bool pinned_memory_source::is_host_accessible() const noexcept
{
	// Page locked memory lives in the host, which is the whole point of it.
	return true;
}

std::size_t pinned_memory_source::get_base_alignment() const noexcept
{
	// Page-locked memory is page-aligned in practice, but only the same amount
	// that every other CUDA allocation guarantees is documented.
	return REXLIB_CUDA_ALLOCATION_ALIGN_BYTES;
}

} // namespace cuda
} // namespace rexlib
