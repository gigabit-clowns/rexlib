// SPDX-License-Identifier: GPL-3.0-only

#include "memory_block_pool.hpp"

#include "memory_heap.hpp"

#include "../../logger.hpp"

#include <rexlib/core/hardware/command_timeline.hpp>
#include <rexlib/core/platform/assert.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace cuda
{

bool memory_block_pool::free_memory_block_compare::operator()(
	const memory_block &lhs,
	const memory_block &rhs
) const noexcept
{
	return is_less(get_key(lhs), get_key(rhs));
}

bool memory_block_pool::free_memory_block_compare::operator()(
	const memory_block &lhs,
	const key_type &rhs
) const noexcept
{
	return is_less(get_key(lhs), rhs);
}

bool memory_block_pool::free_memory_block_compare::operator()(
	const key_type &lhs,
	const memory_block &rhs
) const noexcept
{
	return is_less(lhs, get_key(rhs));
}

memory_block_pool::free_memory_block_compare::key_type
memory_block_pool::free_memory_block_compare::get_key(
	const memory_block &block
) noexcept
{
	return key_type(
		block.get_pending().get_timeline().get(),
		block.get_size()
	);
}

bool memory_block_pool::free_memory_block_compare::is_less(
	const key_type &lhs,
	const key_type &rhs
) noexcept
{
	// Built on std::less rather than on operator<, since comparing unrelated
	// pointers with the latter is unspecified.
	if (lhs.first != rhs.first)
	{
		return std::less<const command_timeline*>()(lhs.first, rhs.first);
	}

	return lhs.second < rhs.second;
}



memory_block_pool::memory_block_pool() noexcept
	: m_size(0)
{
}

memory_block_pool::~memory_block_pool()
{
	if (m_blocks.size() != m_free_blocks.size())
	{
		REXLIB_CUDA_LOG_ERROR(
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
	const command_timeline *timeline
) noexcept
{
	auto *result = find_in_timeline(size, timeline);

	if (!result && timeline)
	{
		// Nothing runs against a block that carries an empty token, so it is
		// as free to take as one this queue only has to run in order for.
		result = find_in_timeline(size, nullptr);
	}

	return result;
}

memory_block* memory_block_pool::find_foreign_block(
	std::size_t size,
	const command_timeline *timeline
) noexcept
{
	memory_block *result = nullptr;

	// One lower bound per timeline that has free blocks. Walking the set
	// timeline by timeline rather than block by block keeps this proportional
	// to how many queues are in play, which is a handful, rather than to how
	// fragmented the pool is.
	auto ite = m_free_blocks.cbegin();
	while (ite != m_free_blocks.cend())
	{
		const auto *candidate_timeline =
			ite->get_pending().get_timeline().get();

		if (candidate_timeline != timeline && candidate_timeline)
		{
			auto *candidate = find_in_timeline(size, candidate_timeline);
			if (candidate &&
			    (!result || candidate->get_size() < result->get_size()))
			{
				result = candidate;
			}
		}

		ite = find_next_timeline(candidate_timeline);
	}

	return result;
}

void memory_block_pool::acquire(memory_block &block) noexcept
{
	REXLIB_ASSERT( block.is_free() );
	m_free_blocks.erase(m_free_blocks.iterator_to(block));
}

void memory_block_pool::release(
	memory_block &block,
	command_token pending
) noexcept
{
	REXLIB_ASSERT( !block.is_free() );
	block.set_pending(std::move(pending));

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
	REXLIB_ASSERT( block.is_free() );
	REXLIB_ASSERT( first_size + second_size == block.get_size() );

	auto first = std::make_unique<memory_block>(
		block.get_pending(),
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

	// Carries an empty token: nothing has ever run against memory the driver
	// has only just handed over.
	auto block = std::make_unique<memory_block>(
		command_token(),
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

void memory_block_pool::enumerate_pending(
	std::vector<command_token> &pending
) const
{
	pending.clear();

	auto ite = m_free_blocks.cbegin();
	while (ite != m_free_blocks.cend())
	{
		const auto *timeline = ite->get_pending().get_timeline().get();
		const auto next = find_next_timeline(timeline);

		if (timeline)
		{
			// The blocks of a timeline are sorted by size, so the latest of
			// their tokens can be any of them.
			const auto latest = std::max_element(
				ite, next,
				[] (const memory_block &lhs, const memory_block &rhs) noexcept
				{
					return lhs.get_pending().get_id() <
					       rhs.get_pending().get_id();
				}
			);
			pending.push_back(latest->get_pending());
		}

		ite = next;
	}
}

void memory_block_pool::reset_pending() noexcept
{
	// Emptying a token rewrites the key a free block is sorted by, so they
	// all have to leave the set rather than be edited in place. They wait in
	// another one, which keeps them answering that they are free and gives
	// the merge below somewhere to erase them from.
	free_memory_block_set_type unbound;
	m_free_blocks.clear_and_dispose(
		[&unbound] (memory_block *block) noexcept
		{
			block->set_pending(command_token());
			unbound.insert(*block);
		}
	);

	// Merging what the timelines were keeping apart is the whole point of
	// emptying the tokens, and it can only be done once every one is empty.
	// Walks the blocks in address order, absorbing each run of free
	// neighbours into the block it starts at.
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

memory_block* memory_block_pool::find_in_timeline(
	std::size_t size,
	const command_timeline *timeline
) noexcept
{
	// Blocks are ordered by timeline first and by size second, so the first
	// one that is not smaller than the request is the best fit, as long as
	// its token is still of the timeline that was asked for.
	const auto ite = m_free_blocks.lower_bound(
		free_memory_block_compare::key_type(timeline, size),
		free_memory_block_compare()
	);

	if (ite == m_free_blocks.end() ||
	    ite->get_pending().get_timeline().get() != timeline)
	{
		return nullptr;
	}

	REXLIB_ASSERT( ite->get_size() >= size );
	return &(*ite);
}

memory_block_pool::free_memory_block_set_type::const_iterator
memory_block_pool::find_next_timeline(
	const command_timeline *timeline
) const noexcept
{
	return m_free_blocks.upper_bound(
		free_memory_block_compare::key_type(
			timeline,
			std::numeric_limits<std::size_t>::max()
		),
		free_memory_block_compare()
	);
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

	keep_later_pending(block, *next_block);
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

	keep_later_pending(block, *previous_block);
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
	// Two blocks only add up to one if a single token says what has to be
	// waited for before taking the result, which the later of two tokens of
	// one timeline does. Merging across timelines would silently hand out
	// memory another queue may still be running against.
	return neighbour.get_heap() == block.get_heap() &&
	       neighbour.get_pending().get_timeline() ==
	       block.get_pending().get_timeline() &&
	       neighbour.is_free();
}

void memory_block_pool::keep_later_pending(
	memory_block &block,
	const memory_block &other
) noexcept
{
	// Only called for tokens of one timeline. Between two empty tokens there
	// is nothing to choose.
	if (!other.get_pending().is_empty() &&
	    other.get_pending().get_id() > block.get_pending().get_id())
	{
		block.set_pending(other.get_pending());
	}
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
} // namespace rexlib
