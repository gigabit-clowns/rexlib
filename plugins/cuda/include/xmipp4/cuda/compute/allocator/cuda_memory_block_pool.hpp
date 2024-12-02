#pragma once

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
 * @file cuda_memory_block_pool.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines memory allocation caching data structures.
 * @date 2024-11-04
 * 
 */

#include "cuda_memory_block.hpp"
#include "cuda_memory_block_usage_tracker.hpp"

#include <map>

namespace xmipp4 
{
namespace compute
{

class cuda_memory_block_context;
class cuda_device_queue;

/**
 * @brief Collection of ordered cuda_memory_block-s with a context.
 * 
 * Due to the comparison mechanism used, items are guaranteed to be
 * ordered first by their queue, then by their size and finally
 * by their data pointer.
 * 
 */
using cuda_memory_block_pool = std::map<cuda_memory_block,
                                        cuda_memory_block_context,
                                        cuda_memory_block_less >;


/**
 * @brief Control block to manage the context of memory blocks in
 * a pool.
 * 
 */
class cuda_memory_block_context
{
public:
    using iterator = cuda_memory_block_pool::iterator;

    /**
     * @brief Construct a new cuda memory block context
     * 
     * @param prev Iterator to the previous partition of a common allocation. 
     * May be null.
     * @param next Iterator to the next partition of a common allocation. 
     * May be null.
     * @param free Wether the block is available.
     * 
     */
    cuda_memory_block_context(iterator prev, 
                              iterator next, 
                              bool free ) noexcept;
    cuda_memory_block_context(const cuda_memory_block_context &other) = default;
    cuda_memory_block_context(cuda_memory_block_context &&other) = default;
    ~cuda_memory_block_context() = default;

    cuda_memory_block_context& 
    operator=(const cuda_memory_block_context &other) = default;
    cuda_memory_block_context& 
    operator=(cuda_memory_block_context &&other) = default;

    /**
     * @brief Get the usage tracker object
     * 
     * @return cuda_memory_block_usage_tracker& The usage tracker object.
     */
    cuda_memory_block_usage_tracker& get_usage_tracker() noexcept;

    /**
     * @brief Set the previous partition.
     * 
     * @param prev Iterator to the previous partition of a common allocation. 
     * May be null.
     * 
     */
    void set_previous_block(iterator prev) noexcept;

    /**
     * @brief Get the previous partition.
     * 
     * @return iterator Iterator to the previous partition of a common 
     * allocation. May be null.
     * 
     */
    iterator get_previous_block() const noexcept;

    /**
     * @brief Set the next partition.
     * 
     * @param next Iterator to the next partition of a common allocation. 
     * May be null.
     * 
     */
    void set_next_block(iterator next) noexcept;

    /**
     * @brief Get the next partition.
     * 
     * @return iterator Iterator to the next partition of a common 
     * allocation. May be null.
     * 
     */
    iterator get_next_block() const noexcept;

    /**
     * @brief Set wether the block is available.
     * 
     * @param free true if it is available, false otherwise.
     */
    void set_free(bool free);

    /**
     * @brief Check wether the block is available.
     * 
     * @return true Block is free.
     * @return false Block is occupied.
     */
    bool is_free() const noexcept;

private:
    cuda_memory_block_usage_tracker m_usage_tracker;
    iterator m_prev;
    iterator m_next;
    bool m_free;

}; 


/**
 * @brief Check if a block is partitioned.
 * 
 * A block is considered to be partitioned if it has a previous or next
 * partition iterator set to a non-empty value.
 * 
 * @param block Block to be checked.
 * @return true Block is partitioned.
 * @return false Block is not partitioned.
 * 
 */
bool is_partition(const cuda_memory_block_context &block) noexcept;

/**
 * @brief Check if a block can be merged to.
 * 
 * A block can be merged if it is free.
 * 
 * @param ite Iterator to the block to be checked. May be null.
 * @return true Item pointed by the iterator is free.
 * @return false Item pointed by the iterator is occupied or the 
 * provided iterator is null.
 * 
 */
bool is_mergeable(cuda_memory_block_pool::iterator ite) noexcept;

/**
 * @brief Update links for the next partition to ensure consistency.
 * 
 * If the provided element has a next partition, the previous partition
 * is updated on the next partition. Otherwise does nothing.
 * 
 * @param ite Iterator to a block. Must be dereferenceable.
 * 
 */
void update_forward_link(cuda_memory_block_pool::iterator ite) noexcept;

/**
 * @brief Update links for the previous partition to ensure consistency.
 * 
 * If the provided element has a previous partition, the next partition
 * is updated on the previous partition. Otherwise does nothing.
 * 
 * @param ite Iterator to a block. Must be dereferenceable.
 * 
 */
void update_backward_link(cuda_memory_block_pool::iterator ite) noexcept;

/**
 * @brief Update links for the previous and next partitions to ensure 
 * consistency.
 * 
 * @param ite Iterator to a block. Must be dereferenceable.
 * 
 * @see update_forward_link
 * @see update_backward_link
 * 
 */
void update_links(cuda_memory_block_pool::iterator ite) noexcept;

/**
 * @brief Check for consistency in the next partition.
 * 
 * The forward link is consistent if the previous block of next block of 
 * the iterator is the iterator itself. If the next block is null, it 
 * is considered to be consistent.
 * 
 * @param ite Iterator to a block. Must be dereferenceable.
 * @return true Forward link is consistent.
 * @return false Forward link is not consistent. Thus, an error ocurred.
 * 
 */
bool check_forward_link(cuda_memory_block_pool::iterator ite) noexcept;

/**
 * @brief Check for consistency in the previous partition.
 * 
 * The backward link is consistent if the next block of the previous block of 
 * the iterator is the iterator itself. If the previous block is null, it is 
 * considered to be consistent.
 * 
 * @param ite Iterator to a block. Must be dereferenceable.
 * @return true Backward link is consistent.
 * @return false Backward link is not consistent. Thus, an error ocurred.
 * 
 */
bool check_backward_link(cuda_memory_block_pool::iterator ite) noexcept;

/**
 * @brief Check for consistency in the previous and next partitions.
 * 
 * @param ite Iterator to a block. Must be dereferenceable.
 * @return true Forward and backward links are consistent.
 * @return false Forward and backward links are not consistent. 
 * Thus, an error ocurred.
 * 
 */
bool check_links(cuda_memory_block_pool::iterator ite) noexcept;

/**
 * @brief Find a candidate block.
 * 
 * If found, the returned block will be at least of the requested size
 * and with the requested block id.
 * 
 * @param blocks Collection of blocks.
 * @param size Minimum size of the block.
 * @param alignment Minumum alignment of the data pointer of the block.
 * @param queue_id Queue id of the block.
 * @return cuda_memory_block_pool::iterator Iterator the candidate block.
 * blocks.end() if none was found.
 * 
 */
cuda_memory_block_pool::iterator 
find_suitable_block(cuda_memory_block_pool &blocks, 
                    std::size_t size,
                    std::size_t alignment,
                    const cuda_device_queue *queue );

/**
 * @brief Partition the requested block if necessary.
 * 
 * The block is only partitioned if the remaining bytes exceed the 
 * provided threshold.
 * 
 * @param blocks Collection of blocks.
 * @param ite Iterator to the candidate block. Must be dereferenceable.
 * @param size Requested size.
 * @param threshold Minimum size excess to partition the block.
 * @return cuda_memory_block_pool::iterator If not partitioning,
 * an same as ite. When partitioning, iterator to the first partition
 * of the input block which will have the requested size.
 * 
 */
cuda_memory_block_pool::iterator 
consider_partitioning_block(cuda_memory_block_pool &blocks,
                            cuda_memory_block_pool::iterator ite,
                            std::size_t size,
                            std::size_t threshold );

/**
 * @brief Partition a block in two blocks.
 * 
 * @param blocks Collection of blocks.
 * @param ite Iterator to the block to be partitioned. Must be dereferenceable.
 * @param size Size of the first partition.
 * @param remaining Size of the second partition.
 * @return cuda_memory_block_pool::iterator Iterator to the first partition
 * of the input block.
 * 
 */
cuda_memory_block_pool::iterator 
partition_block(cuda_memory_block_pool &blocks,
                cuda_memory_block_pool::iterator ite,
                std::size_t size,
                std::size_t remaining );

/**
 * @brief Merge adjacent blocks if necessary.
 * 
 * Forward and/or backward blocks are merged if they are free.
 * 
 * @param blocks Collection of blocks.
 * @param ite Iterator to the candidate block. Must be dereferenceable.
 * @return cuda_memory_block_pool::iterator Input iterator if not merged.
 * Iterator to the merged block when merged.
 * 
 */
cuda_memory_block_pool::iterator 
consider_merging_block(cuda_memory_block_pool &blocks,
                       cuda_memory_block_pool::iterator ite );

/**
 * @brief Merge two blocks.
 * 
 * @param blocks Collection of blocks.
 * @param first Iterator to the first block to be merged. 
 * Must be dereferenceable.
 * @param second Iterator to the second block to be merged. 
 * Must be dereferenceable.
 * @return cuda_memory_block_pool::iterator Iterator to the merged blocks.
 * 
 */
cuda_memory_block_pool::iterator 
merge_blocks(cuda_memory_block_pool &blocks,
             cuda_memory_block_pool::iterator first,
             cuda_memory_block_pool::iterator second );


/**
 * @brief Merge three blocks.
 * 
 * @param blocks Collection of blocks.
 * @param first Iterator to the first block to be merged. 
 * Must be dereferenceable.
 * @param second Iterator to the second block to be merged. 
 * Must be dereferenceable.
 * @param third Iterator to the third block to be merged. 
 * Must be dereferenceable.
 * @return cuda_memory_block_pool::iterator Iterator to the merged blocks.
 * 
 */
cuda_memory_block_pool::iterator 
merge_blocks(cuda_memory_block_pool &blocks,
             cuda_memory_block_pool::iterator first,
             cuda_memory_block_pool::iterator second,
             cuda_memory_block_pool::iterator third );

/**
 * @brief Allocate a new block.
 * 
 * @tparam Allocator class with allocate() and deallocate() methods.
 * @param blocks Collection of blocks.
 * @param allocator The allocator used for creating a new block.
 * @param size Requested block size.
 * @param queue_id Queue where the block belongs to.
 * @return cuda_memory_block_pool::iterator Iterator to the newly allocated
 * block. blocks.end() if the allocation fails.
 * 
 */
template <typename Allocator>
cuda_memory_block_pool::iterator create_block(cuda_memory_block_pool &blocks,
                                              Allocator& allocator,
                                              std::size_t size,
                                              const cuda_device_queue *queue );

/**
 * @brief Request a suitable block.
 *
 * A suitable and free block is searched in the pool if none is found,
 * it is requested from the allocator. 
 *  
 * @tparam Allocator class with allocate() and deallocate() methods.
 * @param blocks Collection of blocks.
 * @param allocator The allocator used for creating a new block.
 * @param size Requested block size.
 * @param alignment Minumum alignment of the data pointer of the block.
 * @param queue_id Queue where the block belongs to.
 * @param partition_min_size Minimum remaining size on a block to consider
 * partitioning it.
 * @param create_size_step Rounding step when considering to create a new block.
 * @return cuda_memory_block_pool::iterator iterator to a suitable block.
 * blocks.end() when failure.
 * 
 */
template <typename Allocator>
cuda_memory_block_pool::iterator 
allocate_block(cuda_memory_block_pool &blocks, 
               const Allocator &allocator, 
               std::size_t size,
               std::size_t alignment,
               const cuda_device_queue *queue,
               std::size_t partition_min_size,
               std::size_t create_size_step );

/**
 * @brief Return a block to the pool.
 * 
 * This marks the provided block as free. If possible, it merges it with the
 * neighboring blocks.
 * 
 * @param blocks Collection of blocks.
 * @param ite Iterator to the block to be returned. 
 * It must belong to the provided pool.
 * 
 */
void deallocate_block(cuda_memory_block_pool &blocks, 
                      cuda_memory_block_pool::iterator ite );

/**
 * @brief Release free blocks when possible.
 * 
 * All free blocks that are not partitioned are returned to the allocator.
 * 
 * @tparam Allocator class with allocate() and deallocate() methods.
 * @param blocks Collection of blocks.
 * @param allocator The allocator used for creating a new block.
 * 
 */
template <typename Allocator>
void release_blocks(cuda_memory_block_pool &blocks, Allocator &allocator);

} // namespace compute
} // namespace xmipp4

#include "cuda_memory_block_pool.inl"
