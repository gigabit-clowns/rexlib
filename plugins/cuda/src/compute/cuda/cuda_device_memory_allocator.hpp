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
 * @file cuda_device_memory_allocator.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines the compute::cuda_device_memory_allocator interface
 * @date 2024-10-31
 * 
 */

#include "allocator/cuda_device_malloc.hpp"
#include "allocator/cuda_memory_cache.hpp"

#include <xmipp4/core/compute/device_memory_allocator.hpp>

#include <map>
#include <forward_list>

namespace xmipp4 
{
namespace compute
{

class cuda_device_queue;
class cuda_device_event;



class cuda_device_memory_allocator
    : public device_memory_allocator
{
public:
    using event_list = std::forward_list<cuda_device_event>;

    explicit cuda_device_memory_allocator(int device_id);
    cuda_device_memory_allocator(const cuda_device_memory_allocator &other) = default;
    cuda_device_memory_allocator(cuda_device_memory_allocator &&other) = default;
    virtual ~cuda_device_memory_allocator() = default;

    cuda_device_memory_allocator&
    operator=(const cuda_device_memory_allocator &other) = default;
    cuda_device_memory_allocator&
    operator=(cuda_device_memory_allocator &&other) = default;

    std::unique_ptr<device_buffer> 
    create_buffer(numerical_type type, 
                  std::size_t count, device_queue &queue ) final;

    std::shared_ptr<device_buffer> 
    create_buffer_shared(numerical_type type, 
                         std::size_t count, 
                         device_queue &queue ) final;

    const cuda_memory_block& allocate(numerical_type type, 
                                      std::size_t count,
                                      cuda_device_queue &queue );
    void deallocate(const cuda_memory_block &block);

private:
    cuda_device_malloc m_allocator;
    cuda_memory_cache m_cache;
    event_list m_event_pool;
    std::map<cuda_memory_block, event_list, cuda_memory_block_size_less> m_pending_free;
    

    void process_pending_free();
    void record_events(const cuda_memory_block &block,
                       std::vector<std::reference_wrapper<cuda_device_queue>> queues);
    void pop_completed_events(event_list &events);

}; 

} // namespace compute
} // namespace xmipp4
