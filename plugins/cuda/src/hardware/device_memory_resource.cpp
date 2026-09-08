// SPDX-License-Identifier: GPL-3.0-only

#include "device_memory_resource.hpp"

#include "caching_allocator/caching_memory_allocator.hpp"
#include "caching_allocator/memory_block_allocator.hpp"
#include "caching_allocator/device_memory_source.hpp"
#include "caching_allocator/pooled_event_recorder.hpp"

namespace rexlib
{
namespace cuda
{

device_memory_resource::device_memory_resource(int ordinal)
	: m_ordinal(ordinal)
{
}

device_memory_resource::~device_memory_resource() = default;

int device_memory_resource::get_ordinal() const noexcept
{
	return m_ordinal;
}

memory_resource_kind device_memory_resource::get_kind() const noexcept
{
	return memory_resource_kind::device_local;
}

std::shared_ptr<memory_allocator>
device_memory_resource::create_allocator() const
{
	return std::make_shared<caching_memory_allocator>(
		*this,
		std::make_shared<memory_block_allocator>(
			std::make_unique<device_memory_source>(m_ordinal),
			std::make_unique<pooled_event_recorder>()
		)
	);
}

} // namespace cuda
} // namespace rexlib
