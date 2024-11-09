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
 * @brief Defines memory allocation data structures.
 * @date 2024-11-04
 * 
 */

#include "cuda_memory_block.hpp"

#include <map>

namespace xmipp4 
{
namespace compute
{

class cuda_memory_block_context;

using cuda_memory_block_pool = std::map<cuda_memory_block, 
                                        cuda_memory_block_context, 
                                        cuda_memory_block_size_less >;

class cuda_memory_block_context
{
public:
    using iterator = cuda_memory_block_pool::iterator;

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

    void set_previous_block(iterator prev) noexcept;
    iterator get_previous_block() const noexcept;

    void set_next_block(iterator next) noexcept;
    iterator get_next_block() const noexcept;

    void set_free(bool free);
    bool is_free() const noexcept;

private:
    iterator m_prev;
    iterator m_next;
    bool m_free;

}; 


bool is_partition(const cuda_memory_block_context &block) noexcept;
bool can_merge(cuda_memory_block_pool::iterator ite) noexcept;
void update_forward_link(cuda_memory_block_pool::iterator ite) noexcept;
void update_backward_link(cuda_memory_block_pool::iterator ite) noexcept;
void update_links(cuda_memory_block_pool::iterator ite) noexcept;
bool check_forward_link(cuda_memory_block_pool::iterator ite) noexcept;
bool check_backward_link(cuda_memory_block_pool::iterator ite) noexcept;
bool check_links(cuda_memory_block_pool::iterator ite) noexcept;

cuda_memory_block_pool::iterator 
find_suitable_block(cuda_memory_block_pool &blocks, 
                    std::size_t size,
                    std::size_t queue_id );

cuda_memory_block_pool::iterator 
consider_partitioning_block(cuda_memory_block_pool &blocks,
                            cuda_memory_block_pool::iterator ite,
                            std::size_t size,
                            std::size_t threshold );

cuda_memory_block_pool::iterator 
partition_block(cuda_memory_block_pool &blocks,
                cuda_memory_block_pool::iterator ite,
                std::size_t size,
                std::size_t remaining );

cuda_memory_block_pool::iterator 
consider_merging_block(cuda_memory_block_pool &blocks,
                       cuda_memory_block_pool::iterator ite );

cuda_memory_block_pool::iterator 
merge_blocks(cuda_memory_block_pool &blocks,
             cuda_memory_block_pool::iterator first,
             cuda_memory_block_pool::iterator second );
cuda_memory_block_pool::iterator 
merge_blocks(cuda_memory_block_pool &blocks,
             cuda_memory_block_pool::iterator first,
             cuda_memory_block_pool::iterator second,
             cuda_memory_block_pool::iterator third );

template <typename Allocator>
cuda_memory_block_pool::iterator create_block(cuda_memory_block_pool &blocks,
                                              Allocator& allocator,
                                              std::size_t size,
                                              std::size_t queue_id );

template <typename Allocator>
const cuda_memory_block* allocate_block(cuda_memory_block_pool &blocks, 
                                        const Allocator &allocator, 
                                        std::size_t size,
                                        std::size_t queue_id,
                                        std::size_t partition_min_size,
                                        std::size_t create_size_step );
void deallocate_block(cuda_memory_block_pool &blocks, 
                      const cuda_memory_block &block);


template <typename Allocator>
void release_blocks(cuda_memory_block_pool &blocks, Allocator &allocator);

} // namespace compute
} // namespace xmipp4

#include "cuda_memory_block_pool.inl"
