// SPDX-License-Identifier: GPL-3.0-only

#include "memory_block.hpp"

#include "memory_heap.hpp"

#include <rexlib/core/memory/align.hpp>

namespace rexlib
{
namespace cuda
{

inline
memory_block::memory_block(
	const queue_handle &queue,
	std::size_t size,
	memory_heap *heap,
	std::size_t offset
) noexcept
	: m_queue(queue)
	, m_size(size)
	, m_heap(heap)
	, m_offset(offset)
{
}

inline
void memory_block::set_queue(const queue_handle &queue) noexcept
{
	m_queue = queue;
}

inline
const queue_handle& memory_block::get_queue() const noexcept
{
	return m_queue;
}

inline
void memory_block::set_size(std::size_t size) noexcept
{
	m_size = size;
}

inline
std::size_t memory_block::get_size() const noexcept
{
	return m_size;
}

inline
memory_heap* memory_block::get_heap() const noexcept
{
	return m_heap;
}

inline
void memory_block::set_offset(std::size_t offset) noexcept
{
	m_offset = offset;
}

inline
std::size_t memory_block::get_offset() const noexcept
{
	return m_offset;
}

inline
void* memory_block::get_data() const noexcept
{
	return offset_bytes(m_heap->get_data(), m_offset);
}

inline
bool memory_block::is_free() const noexcept
{
	return free_block_set_hook.is_linked();
}

} // namespace cuda
} // namespace rexlib
