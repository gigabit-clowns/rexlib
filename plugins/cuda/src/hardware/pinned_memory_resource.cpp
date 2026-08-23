// SPDX-License-Identifier: GPL-3.0-only

#include "pinned_memory_resource.hpp"

#include "caching_allocator/caching_memory_allocator.hpp"
#include "caching_allocator/memory_block_allocator.hpp"
#include "caching_allocator/pinned_memory_source.hpp"
#include "caching_allocator/pooled_event_recorder.hpp"

#include <cuda_runtime.h>

namespace xmipp4
{
namespace cuda
{

// Page locked memory is what the devices compute out of only where every one
// of them shares the host's, and a staging area to transfer through otherwise.
// A machine with no device at all has nothing to share with, and one mixing
// both sorts of device is answered conservatively: those that do share lose an
// optimization, rather than every buffer being promised an accessibility that
// does not hold for all of them.
static memory_resource_kind query_pinned_memory_kind() noexcept
{
	int count;
	if (cudaGetDeviceCount(&count) != cudaSuccess || count == 0)
	{
		return memory_resource_kind::host_staging;
	}

	for (int ordinal = 0; ordinal < count; ++ordinal)
	{
		cudaDeviceProp properties;
		if (cudaGetDeviceProperties(&properties, ordinal) != cudaSuccess)
		{
			return memory_resource_kind::host_staging;
		}

		if (!properties.integrated)
		{
			return memory_resource_kind::host_staging;
		}
	}

	return memory_resource_kind::unified;
}



pinned_memory_resource::pinned_memory_resource()
	: m_kind(query_pinned_memory_kind())
{
}

pinned_memory_resource::~pinned_memory_resource() = default;

memory_resource_kind pinned_memory_resource::get_kind() const noexcept
{
	return m_kind;
}

std::shared_ptr<memory_allocator>
pinned_memory_resource::create_allocator() const
{
	// Mapping page locked memory into the device address space is only
	// worth what it costs where the device would otherwise have to
	// transfer it, which is exactly where it does not share it.
	const bool mapped = (m_kind == memory_resource_kind::unified);
	return std::make_shared<caching_memory_allocator>(
		*this,
		std::make_shared<memory_block_allocator>(
			std::make_unique<pinned_memory_source>(mapped),
			std::make_unique<pooled_event_recorder>()
		)
	);
}

const pinned_memory_resource& pinned_memory_resource::get()
{
	// Built on first use rather than at load time, since it asks the driver
	// what devices there are, and a plugin is loaded long before anyone has
	// said they want one.
	static const pinned_memory_resource instance;
	return instance;
}

} // namespace cuda
} // namespace xmipp4
