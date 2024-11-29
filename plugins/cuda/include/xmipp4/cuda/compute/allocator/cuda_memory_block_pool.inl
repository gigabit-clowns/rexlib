/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

/**
 * @file cuda_memory_block_pool.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_memory_block_pool.hpp
 * @date 2024-11-04
 * 
 */

#include "cuda_memory_block_pool.hpp"

#include <xmipp4/core/memory/align.hpp>
#include <xmipp4/core/platform/assert.hpp>

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <tuple>

namespace xmipp4
{
namespace compute
{

inline
std::size_t cuda_memory_block_pool::size() const noexcept
{
    return m_items.size();
}

inline
cuda_memory_block_pool::iterator cuda_memory_block_pool::begin() noexcept
{
    return m_items.begin();
}

inline
cuda_memory_block_pool::iterator cuda_memory_block_pool::end() noexcept
{
    return m_items.end();
}

inline
cuda_memory_block_pool::const_iterator 
cuda_memory_block_pool::begin() const noexcept
{
    return m_items.begin();
}

inline
cuda_memory_block_pool::const_iterator 
cuda_memory_block_pool::end() const noexcept
{
    return m_items.end();
}

inline
cuda_memory_block_pool::const_iterator 
cuda_memory_block_pool::cbegin() const noexcept
{
    return m_items.cbegin();
}

inline
cuda_memory_block_pool::const_iterator 
cuda_memory_block_pool::cend() const noexcept
{
    return m_items.cend();
}


template <typename... Args>
inline
cuda_memory_block_pool::iterator cuda_memory_block_pool::emplace(Args&&... args)
{
    iterator result;
    if (m_free_list.empty())
    {
        result = m_free_list.emplace(
            m_free_list.cbegin(),
            std::forward<Args>(args)...
        );
    }
    else
    {
        result = m_free_list.begin();
        *result = value_type(std::forward<Args>(args)...);
    }

    const auto pos = std::lower_bound(
        m_items.cbegin(), m_items.cend(),
        result->first,
        [] (const value_type& lhs, const key_type &rhs)
        {
            return cuda_memory_block_less()(lhs.first, rhs);
        }
    );

    m_items.splice(pos, m_free_list, result);
    return result;
}

inline
cuda_memory_block_pool::iterator 
cuda_memory_block_pool::erase(iterator pos)
{
    const auto result = std::next(pos);
    m_free_list.splice(m_free_list.cend(), m_items, pos);
    return result;
}

inline
cuda_memory_block_pool::iterator 
cuda_memory_block_pool::find(const cuda_memory_block &item)
{
    auto ite = std::lower_bound(
        begin(), end(),
        item,
        [] (const value_type& lhs, const key_type &rhs)
        {
            return cuda_memory_block_less()(lhs.first, rhs);
        }
    );

    if (ite->first.get_data() != item.get_data())
    {
        ite = end();
    }

    return ite;
}





inline
cuda_memory_block_context::cuda_memory_block_context(iterator prev, 
                                                     iterator next, 
                                                     bool free ) noexcept
    : m_prev(prev)
    , m_next(next)
    , m_free(free)
{
}

inline
void cuda_memory_block_context::set_previous_block(iterator prev) noexcept
{
    m_prev = prev;
}

inline
cuda_memory_block_context::iterator 
cuda_memory_block_context::get_previous_block() const noexcept
{
    return m_prev;
}

inline
void cuda_memory_block_context::set_next_block(iterator next) noexcept
{
    m_next = next;
}

inline
cuda_memory_block_context::iterator
cuda_memory_block_context::get_next_block() const noexcept
{
    return m_next;
}

inline
void cuda_memory_block_context::set_free(bool free)
{
    m_free = free;
}

inline
bool cuda_memory_block_context::is_free() const noexcept
{
    return m_free;
}





inline
bool is_partition(const cuda_memory_block_context &block) noexcept
{
    const cuda_memory_block_pool::iterator null;
    return block.get_previous_block() != null ||
           block.get_next_block() != null ;
}

inline
bool is_mergeable(cuda_memory_block_pool::iterator ite) noexcept
{
    bool result;

    if (ite != cuda_memory_block_pool::iterator())
    {
        result = ite->second.is_free();
    }
    else
    {
        result = false;
    }

    return result;
}

inline
void update_forward_link(cuda_memory_block_pool::iterator ite) noexcept
{
    const auto next = ite->second.get_next_block();
    if (next != cuda_memory_block_pool::iterator())
    {
        next->second.set_previous_block(ite);
    }
}

inline
void update_backward_link(cuda_memory_block_pool::iterator ite) noexcept
{
    const auto prev = ite->second.get_previous_block();
    if (prev != cuda_memory_block_pool::iterator())
    {
        prev->second.set_next_block(ite);
    }
}

inline
void update_links(cuda_memory_block_pool::iterator ite) noexcept
{
    update_backward_link(ite);
    update_forward_link(ite);
}

inline
bool check_forward_link(cuda_memory_block_pool::iterator ite)  noexcept
{
    bool result;

    const auto next = ite->second.get_next_block();
    if (next != cuda_memory_block_pool::iterator())
    {
        result = next->second.get_previous_block() == ite;
    }
    else
    {
        result = true;
    }

    return result;
}

inline
bool check_backward_link(cuda_memory_block_pool::iterator ite) noexcept
{
    bool result;

    const auto prev = ite->second.get_previous_block();
    if (prev != cuda_memory_block_pool::iterator())
    {
        result = prev->second.get_next_block() == ite;
    }
    else
    {
        result = true;
    }

    return result;
}

inline
bool check_links(cuda_memory_block_pool::iterator ite) noexcept
{
    return check_backward_link(ite) && check_forward_link(ite);
}

inline
cuda_memory_block_pool::iterator 
find_suitable_block(cuda_memory_block_pool &blocks, 
                    std::size_t size,
                    const cuda_device_queue *queue )
{
    // Assuming that the blocks are ordered according to their queue reference
    // first and then their sizes, find a feasible range where all elements
    // belong to a queue and have a size greater or equal to the requested
    // size.
    const auto first = std::lower_bound(
        blocks.begin(), blocks.end(),
        std::make_tuple(queue, size),
        [] (const auto &item, const auto &key2) -> bool
        {
            const auto &block = item.first;
            const auto key1 = 
                std::make_tuple(block.get_queue(), block.get_size());
            return key1 < key2;
        }
    );

    const auto last = std::upper_bound(
        first, blocks.end(),
        queue,
        [] (const cuda_device_queue *key1, const auto &item) -> bool
        {
            const auto key2 = item.first.get_queue();
            return key1 < key2;
        }
    );

    // In the feasible range of blocks, find the smallest free block.
    auto ite = std::find_if(
        first, last,
        [] (const auto &item) -> bool
        {
            return item.second.is_free();
        }
    );

    return ite != last ? ite : blocks.end();
}

inline
cuda_memory_block_pool::iterator 
consider_partitioning_block(cuda_memory_block_pool &blocks,
                            cuda_memory_block_pool::iterator ite,
                            std::size_t size,
                            std::size_t threshold )
{
    const auto remaining = ite->first.get_size() - size;
    if (remaining >= threshold)
    {
        ite = partition_block(blocks, ite, size, remaining);
    }

    return ite;
}

inline
cuda_memory_block_pool::iterator 
partition_block(cuda_memory_block_pool &blocks,
                cuda_memory_block_pool::iterator ite,
                std::size_t size,
                std::size_t remaining )
{
    const auto queue = ite->first.get_queue();
    const auto prev = ite->second.get_previous_block();
    const auto next = ite->second.get_next_block();

    const auto first = blocks.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(
            ite->first.get_data(), 
            size, 
            queue
        ),
        std::forward_as_tuple(
            prev,
            cuda_memory_block_pool::iterator(), // To be set
            ite->second.is_free()
        )
    );
    const auto second = blocks.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(
            memory::offset_bytes(ite->first.get_data(), size), 
            remaining, 
            queue
        ),
        std::forward_as_tuple(
            first,
            next,
            ite->second.is_free()
        )
    );
    
    first->second.set_next_block(second);
    update_backward_link(first);
    update_forward_link(second);

    // Remove old block
    blocks.erase(ite);

    XMIPP4_ASSERT( check_links(first) );
    XMIPP4_ASSERT( check_links(second) );

    return first;
}

inline
cuda_memory_block_pool::iterator
consider_merging_block(cuda_memory_block_pool &blocks, 
                       cuda_memory_block_pool::iterator ite)
{
    const auto prev = ite->second.get_previous_block();
    const auto merge_prev = is_mergeable(prev);
    const auto next = ite->second.get_next_block();
    const auto merge_next = is_mergeable(next);

    if (merge_prev && merge_next)
    {
        ite = merge_blocks(blocks, prev, ite, next);
    }
    else if (merge_prev)
    {
        ite = merge_blocks(blocks, prev, ite);
    }
    else if (merge_next)
    {   
        ite = merge_blocks(blocks, ite, next);
    }

    return ite;
}

inline
cuda_memory_block_pool::iterator
merge_blocks(cuda_memory_block_pool &blocks,
             cuda_memory_block_pool::iterator first,
             cuda_memory_block_pool::iterator second )
{
    const auto data = first->first.get_data();
    const auto size = first->first.get_size() +
                      second->first.get_size() ;
    const auto queue = first->first.get_queue();
    const auto prev = first->second.get_previous_block();
    const auto next = second->second.get_next_block();

    const auto ite = blocks.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(data, size, queue),
        std::forward_as_tuple(prev, next, true)
    );
    update_links(ite);

    blocks.erase(first);
    blocks.erase(second);

    XMIPP4_ASSERT( check_links(ite) );
    return ite;
}

inline
cuda_memory_block_pool::iterator
merge_blocks(cuda_memory_block_pool &blocks,
             cuda_memory_block_pool::iterator first,
             cuda_memory_block_pool::iterator second,
             cuda_memory_block_pool::iterator third )
{
    const auto data = first->first.get_data();
    const auto size = first->first.get_size() +
                      second->first.get_size() +
                      third->first.get_size() ;
    const auto queue = first->first.get_queue();
    const auto prev = first->second.get_previous_block();
    const auto next = third->second.get_next_block();

    const auto ite = blocks.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(data, size, queue),
        std::forward_as_tuple(prev, next, true)
    );
    update_links(ite);

    blocks.erase(first);
    blocks.erase(second);
    blocks.erase(third);

    XMIPP4_ASSERT( check_links(ite) );
    return ite;
}

template <typename Allocator>
inline
cuda_memory_block_pool::iterator create_block(cuda_memory_block_pool &blocks,
                                              Allocator& allocator,
                                              std::size_t size,
                                              const cuda_device_queue *queue )
{
    cuda_memory_block_pool::iterator result;

    // Try to allocate
    void* data = allocator.allocate(size);
    if (data)
    {
        const cuda_memory_block_pool::iterator null;

        // Add block
        result = blocks.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(data, size, queue),
            std::forward_as_tuple(null, null, true)
        );
    }
    else
    {
        result = blocks.end();
    }

    return result;
}

template <typename Allocator>
inline
cuda_memory_block* allocate_block(cuda_memory_block_pool &blocks, 
                                  const Allocator &allocator, 
                                  std::size_t size,
                                  const cuda_device_queue *queue,
                                  std::size_t partition_min_size,
                                  std::size_t create_size_step )
{
    cuda_memory_block *result;

    auto ite = find_suitable_block(blocks, size, queue);
    if (ite == blocks.end())
    {
        const auto create_size = memory::align_ceil(size, create_size_step);
        ite = create_block(blocks, allocator, create_size, queue);
    }

    if (ite != blocks.end())
    {
        ite = consider_partitioning_block(blocks, ite, size, partition_min_size);
        ite->second.set_free(false);
        result = &(ite->first);
    }
    else
    {
        result = nullptr;
    }

    return result;
}

inline
void deallocate_block(cuda_memory_block_pool &blocks, 
                      const cuda_memory_block &block)
{
    auto ite = blocks.find(block);
    if (ite == blocks.end())
    {
        throw std::invalid_argument("Block does not belong to this pool");
    }

    ite->first.reset_extra_queues();
    ite->second.set_free(true);
    ite = consider_merging_block(blocks, ite);
}

template <typename Allocator>
inline
void release_blocks(cuda_memory_block_pool &blocks, Allocator &allocator)
{
    auto ite = blocks.begin();
    while (ite != blocks.cend())
    {
        if(ite->second.is_free() && !is_partition(ite->second))
        {
            allocator.deallocate(
                ite->first.get_data(),
                ite->first.get_size()
            );
            ite = blocks.erase(ite);
        }
        else
        {
            ++ite;
        }
    }
}

} // namespace compute
} // namespace xmipp4
