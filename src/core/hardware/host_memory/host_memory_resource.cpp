// SPDX-License-Identifier: GPL-3.0-only

#include "host_memory_resource.hpp"

#include "host_memory_allocator.hpp"


namespace rex
{

const host_memory_resource host_memory_resource::m_instance;

host_memory_resource::host_memory_resource() noexcept = default;
host_memory_resource::~host_memory_resource() = default;

memory_resource_kind host_memory_resource::get_kind() const noexcept
{
	return memory_resource_kind::host;
}

std::shared_ptr<memory_allocator> host_memory_resource::create_allocator() const
{
	return host_memory_allocator::create();
}

const host_memory_resource& host_memory_resource::get() noexcept
{
	return m_instance;
}

} // namespace rex
