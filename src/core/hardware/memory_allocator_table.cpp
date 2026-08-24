// SPDX-License-Identifier: GPL-3.0-only

#include "memory_allocator_table.hpp"

#include <rex/core/hardware/device.hpp>
#include <rex/core/hardware/memory_resource.hpp>
#include <rex/core/platform/assert.hpp>

#include <unordered_map>

namespace rexlib
{

memory_allocator_table::memory_allocator_table(const device &dev)
{
	std::unordered_map<
		const memory_resource*,
		std::shared_ptr<memory_allocator>
	> allocators;

	for (std::size_t i = 0; i < m_allocators.size(); ++i)
	{
		const auto affinity = static_cast<memory_resource_affinity>(i);
		const auto &resource = dev.get_memory_resource(affinity);
		auto ite = allocators.find(&resource);
		if (ite == allocators.end())
		{
			std::tie(ite, std::ignore) = allocators.emplace(
				&resource,
				resource.create_allocator()
			);
		}

		REXLIB_ASSERT(ite != allocators.end());
		REXLIB_ASSERT(ite->second != nullptr);
		m_allocators[i] = ite->second;
	}
}

std::shared_ptr<memory_allocator>
memory_allocator_table::set_allocator(
	memory_resource_affinity affinity,
	std::shared_ptr<memory_allocator> allocator
) noexcept
{
	std::swap(m_allocators[static_cast<std::size_t>(affinity)], allocator);
	return allocator;
}

const std::shared_ptr<memory_allocator>&
memory_allocator_table::get_allocator(
	memory_resource_affinity affinity
) const noexcept
{
	return m_allocators[static_cast<std::size_t>(affinity)];
}

} // namespace rexlib

