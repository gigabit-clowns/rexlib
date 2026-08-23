// SPDX-License-Identifier: GPL-3.0-only

#include "memory_heap.hpp"

#include "memory_source.hpp"

#include <xmipp4/core/memory/align.hpp>

#include <stdexcept>

namespace xmipp4
{
namespace cuda
{

memory_heap::memory_heap(
	memory_source &source,
	std::size_t size,
	std::size_t alignment
)
	: m_source(&source)
	, m_allocation(nullptr)
	, m_data(nullptr)
	, m_size(size)
{
	if (size == 0)
	{
		throw std::invalid_argument(
			"memory_heap::memory_heap: An empty heap can not be carved into "
			"blocks."
		);
	}

	// Whatever the source aligns more loosely than promised has to be paid for
	// up front, since the base can only be moved forwards.
	const auto base_alignment = source.get_base_alignment();
	const auto padding =
		alignment > base_alignment ? alignment - base_alignment : 0;

	m_allocation = source.allocate(size + padding);
	m_data = align_ceil(m_allocation, alignment);
}

memory_heap::~memory_heap()
{
	m_source->deallocate(m_allocation);
}

void* memory_heap::get_data() const noexcept
{
	return m_data;
}

std::size_t memory_heap::get_size() const noexcept
{
	return m_size;
}

} // namespace cuda
} // namespace xmipp4
