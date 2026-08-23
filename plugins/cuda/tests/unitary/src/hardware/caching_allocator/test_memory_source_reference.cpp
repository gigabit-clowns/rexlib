// SPDX-License-Identifier: GPL-3.0-only

#include "test_memory_source_reference.hpp"

namespace xmipp4
{
namespace cuda
{

memory_source_reference::memory_source_reference(
	memory_source &target
) noexcept
	: m_target(&target)
{
}

memory_source_reference::~memory_source_reference() = default;

void* memory_source_reference::allocate(std::size_t size)
{
	return m_target->allocate(size);
}

void memory_source_reference::deallocate(void *data) noexcept
{
	m_target->deallocate(data);
}

bool memory_source_reference::is_host_accessible() const noexcept
{
	return m_target->is_host_accessible();
}

std::size_t memory_source_reference::get_base_alignment() const noexcept
{
	return m_target->get_base_alignment();
}

} // namespace cuda
} // namespace xmipp4
