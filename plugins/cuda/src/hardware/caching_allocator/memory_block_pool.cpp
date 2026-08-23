// SPDX-License-Identifier: GPL-3.0-only

#include "memory_block_pool.hpp"

#include "memory_heap.hpp"

#include "../../logger.hpp"

#include <xmipp4/core/platform/assert.hpp>

#include <limits>
#include <stdexcept>

namespace xmipp4
{
namespace cuda
{

bool memory_block_pool::free_memory_block_compare::operator()(
	const memory_block &lhs,
	const memory_block &rhs
) const noexcept
{
	const auto &lhs_queue = lhs.get_queue();
	const auto &rhs_queue = rhs.get_queue();

	if (lhs_queue != rhs_queue)
	{
		return lhs_queue < rhs_queue;
	}

	return lhs.get_size() < rhs.get_size();
}



memory_block_pool::memory_block_pool() noexcept
	: m_size(0)
{
}

memory_block_pool::~memory_block_pool()
{
	if (m_blocks.size() != m_free_blocks.size())
	{
		XMIPP4_CUDA_LOG_ERROR(
			"Some blocks belonging to a memory_block_pool were not released "
			"before its destruction."
		);
	}

	m_free_blocks.clear();
	m_blocks.clear_and_dispose(std::default_delete<memory_block>());
	m_heaps.clear();
}

memory_block* memory_block_pool::find_suitable_block(
	std::size_t size,
	const queue_handle &queue
) noexcept
{
	auto *result = find_in_queue(size, queue);

	if (!result && queue)
	{
		// Nothing has ever run against a block that belongs to no queue, so it
		// is as free to take as one of this queue's own.
		result = find_in_queue(size, queue_handle());
	}

	return result;
}

memory_block* memory_block_pool::find_foreign_block(
	std::size_t size,
	const queue_handle &queue
) noexcept
{
	memory_block *result = nullptr;

	// One lower bound per queue that has free blocks. Walking the set queue by
	// queue rather than block by block keeps this proportional to how many
	// queues are in play, which is a handful, rather than to how fragmented
	// the pool is.
	auto ite = m_free_blocks.begin();
	while (ite != m_free_blocks.end())
	{
		const auto &candidate_queue = ite->get_queue();

		if (candidate_queue != queue && candidate_queue)
		{
			auto *candidate = find_in_queue(size, candidate_queue);
			if (candidate &&
			    (!result || candidate->get_size() < result->get_size()))
			{
				result = candidate;
			}
		}

		// Skip to the first block of the next queue.
		const memory_block key(
			candidate_queue,
			std::numeric_limits<std::size_t>::max(),
			nullptr,
			0
		);
		ite = m_free_blocks.upper_bound(key);
	}

	return result;
}

void memory_block_pool::acquire(
	memory_block &block,
	const queue_handle &queue
) noexcept
{
	XMIPP4_ASSERT( block.is_free() );
	m_free_blocks.erase(m_free_blocks.iterator_to(block));
	block.set_queue(queue);
}

void memory_block_pool::release(memory_block &block) noexcept
{
	XMIPP4_ASSERT( !block.is_free() );

	// Merging before the block joins the set keeps the set from having to be
	// searched for a block that is about to grow anyway.
	consider_merging_block(block);
	m_free_blocks.insert(block);
}

std::pair<memory_block*, memory_block*>
memory_block_pool::partition_block(
	memory_block &block,
	std::size_t first_size,
	std::size_t second_size
)
{
	XMIPP4_ASSERT( block.is_free() );
	XMIPP4_ASSERT( first_size + second_size == block.get_size() );

	auto first = std::make_unique<memory_block>(
		block.get_queue(),
		first_size,
		block.get_heap(),
		block.get_offset()
	);

	// The block that was already tracked becomes the second half, so that
	// nothing else has to be touched if making the first one fails.
	auto *second = &block;
	m_free_blocks.erase(m_free_blocks.iterator_to(*second));
	second->set_size(second_size);
	second->set_offset(second->get_offset() + first_size);

	m_blocks.insert(m_blocks.iterator_to(*second), *first);
	m_free_blocks.insert(*first);
	m_free_blocks.insert(*second);

	return std::make_pair(first.release(), second);
}

memory_block* memory_block_pool::register_heap(
	std::unique_ptr<memory_heap> heap
)
{
	if (!heap)
	{
		throw std::invalid_argument(
			"memory_block_pool::register_heap: A heap is required."
		);
	}

	auto *const heap_pointer = heap.get();
	const auto size = heap->get_size();

	// Belongs to no queue: nothing has ever run against memory the driver has
	// only just handed over.
	auto block = std::make_unique<memory_block>(
		queue_handle(),
		size,
		heap_pointer,
		0
	);

	m_heaps.emplace(heap_pointer, std::move(heap));

	m_blocks.push_back(*block);
	m_free_blocks.insert(*block);
	m_size += size;

	return block.release();
}

void memory_block_pool::enumerate_queues(
	std::vector<queue_handle> &queues
) const
{
	queues.clear();

	auto ite = m_free_blocks.begin();
	while (ite != m_free_blocks.end())
	{
		const auto &queue = ite->get_queue();
		if (queue)
		{
			queues.push_back(queue);
		}

		const memory_block key(
			queue,
			std::numeric_limits<std::size_t>::max(),
			nullptr,
			0
		);
		ite = m_free_blocks.upper_bound(key);
	}
}

void memory_block_pool::reset_queues() noexcept
{
	// Unbinding rewrites the key a free block is sorted by, so they all have
	// to leave the set rather than be edited in place. They wait in another
	// one, which keeps them answering that they are free and gives the merge
	// below somewhere to erase them from.
	free_memory_block_set_type unbound;
	m_free_blocks.clear_and_dispose(
		[&unbound] (memory_block *block) noexcept
		{
			block->set_queue(queue_handle());
			unbound.insert(*block);
		}
	);

	// Merging what the queue boundaries were keeping apart is the whole point
	// of unbinding, and it can only be done once every block is unbound. Walks
	// the blocks in address order, absorbing each run of free neighbours into
	// the block it starts at.
	auto ite = m_blocks.begin();
	while (ite != m_blocks.end())
	{
		const auto next = std::next(ite);
		if (ite->is_free() &&
		    next != m_blocks.end() &&
		    next->is_free() &&
		    next->get_heap() == ite->get_heap())
		{
			ite->set_size(ite->get_size() + next->get_size());
			unbound.erase(unbound.iterator_to(*next));
			m_blocks.erase_and_dispose(
				next,
				std::default_delete<memory_block>()
			);
			continue;
		}

		++ite;
	}

	unbound.clear_and_dispose(
		[this] (memory_block *block) noexcept
		{
			m_free_blocks.insert(*block);
		}
	);
}

std::size_t memory_block_pool::release_unused_heaps() noexcept
{
	std::size_t result = 0;

	auto ite = m_free_blocks.begin();
	while (ite != m_free_blocks.end())
	{
		if (is_partition(*ite))
		{
			++ite;
			continue;
		}

		auto *block = &(*ite);
		const auto *heap = block->get_heap();

		result += heap->get_size();
		m_size -= heap->get_size();

		m_blocks.erase(m_blocks.iterator_to(*block));
		ite = m_free_blocks.erase_and_dispose(
			ite,
			std::default_delete<memory_block>()
		);
		m_heaps.erase(heap);
	}

	return result;
}

std::size_t memory_block_pool::get_size() const noexcept
{
	return m_size;
}

std::size_t memory_block_pool::get_acquired_block_count() const noexcept
{
	return m_blocks.size() - m_free_blocks.size();
}

memory_block* memory_block_pool::find_in_queue(
	std::size_t size,
	const queue_handle &queue
) noexcept
{
	// Blocks are ordered by queue first and by size second, so the first one
	// that is not smaller than the request is the best fit, as long as it
	// still belongs to the queue that was asked for.
	const memory_block key(queue, size, nullptr, 0);
	const auto ite = m_free_blocks.lower_bound(key);

	if (ite == m_free_blocks.end() || ite->get_queue() != queue)
	{
		return nullptr;
	}

	XMIPP4_ASSERT( ite->get_size() >= size );
	return &(*ite);
}

void memory_block_pool::consider_merging_block(memory_block &block) noexcept
{
	consider_merging_forwards(block);
	consider_merging_backwards(block);
}

void memory_block_pool::consider_merging_forwards(memory_block &block) noexcept
{
	const auto ite = m_blocks.iterator_to(block);
	const auto next = std::next(ite);
	if (next == m_blocks.end() || !is_mergeable(block, *next))
	{
		return;
	}

	auto *next_block = &(*next);
	const auto merged_size = block.get_size() + next_block->get_size();

	m_free_blocks.erase(m_free_blocks.iterator_to(*next_block));
	m_blocks.erase_and_dispose(next, std::default_delete<memory_block>());
	block.set_size(merged_size);
}

void memory_block_pool::consider_merging_backwards(memory_block &block) noexcept
{
	const auto ite = m_blocks.iterator_to(block);
	if (ite == m_blocks.begin())
	{
		return;
	}

	const auto previous = std::prev(ite);
	if (!is_mergeable(block, *previous))
	{
		return;
	}

	auto *previous_block = &(*previous);
	const auto merged_size = block.get_size() + previous_block->get_size();
	const auto merged_offset = previous_block->get_offset();

	m_free_blocks.erase(m_free_blocks.iterator_to(*previous_block));
	m_blocks.erase_and_dispose(previous, std::default_delete<memory_block>());
	block.set_size(merged_size);
	block.set_offset(merged_offset);
}

bool memory_block_pool::is_mergeable(
	const memory_block &block,
	const memory_block &neighbour
) const noexcept
{
	// Two blocks only add up to one if what a queue has to wait for before
	// taking the result is the same for both halves, which is what belonging
	// to the same queue says. Merging across that would silently hand out
	// memory another queue may still be running against.
	return neighbour.get_heap() == block.get_heap() &&
	       neighbour.get_queue() == block.get_queue() &&
	       neighbour.is_free();
}

bool memory_block_pool::is_partition(const memory_block &block) const noexcept
{
	const auto ite = m_blocks.iterator_to(block);

	const auto next = std::next(ite);
	if (next != m_blocks.end() && next->get_heap() == block.get_heap())
	{
		return true;
	}

	if (ite != m_blocks.begin() &&
	    std::prev(ite)->get_heap() == block.get_heap())
	{
		return true;
	}

	return false;
}

} // namespace cuda
} // namespace xmipp4
