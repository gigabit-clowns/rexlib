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
#include "allocator/cuda_caching_memory_allocator.hpp"

#include <xmipp4/core/compute/device_memory_allocator.hpp>
#include <xmipp4/core/span.hpp>

#include <map>
#include <set>
#include <forward_list>

namespace xmipp4 
{
namespace compute
{

class cuda_device;
class cuda_device_queue;
class cuda_device_buffer;
class cuda_event;

class cuda_device_memory_allocator
    : public device_memory_allocator
{
public:
    explicit cuda_device_memory_allocator(cuda_device &device);
    cuda_device_memory_allocator(const cuda_device_memory_allocator &other) = delete;
    cuda_device_memory_allocator(cuda_device_memory_allocator &&other) = default;
    ~cuda_device_memory_allocator() override = default;

    cuda_device_memory_allocator&
    operator=(const cuda_device_memory_allocator &other) = delete;
    cuda_device_memory_allocator&
    operator=(cuda_device_memory_allocator &&other) = default;

    std::unique_ptr<device_buffer> 
    create_device_buffer(std::size_t size,
                         std::size_t alignment,
                         device_queue &queue ) override;

    std::unique_ptr<cuda_device_buffer> 
    create_device_buffer_impl(std::size_t size,
                              std::size_t alignment,
                              cuda_device_queue &queue );

    std::shared_ptr<device_buffer> 
    create_device_buffer_shared(std::size_t size,
                                std::size_t alignment,
                                device_queue &queue ) override;

    std::shared_ptr<cuda_device_buffer> 
    create_device_buffer_shared_impl(std::size_t size,
                                     std::size_t alignment,
                                     cuda_device_queue &queue );

    const cuda_memory_block& allocate(std::size_t size,
                                      std::size_t alignment,
                                      cuda_device_queue *queue,
                                      cuda_memory_block_usage_tracker **usage_tracker );
    void deallocate(const cuda_memory_block &block);

private:
    cuda_caching_memory_allocator<cuda_device_malloc> m_allocator;

}; 

} // namespace compute
} // namespace xmipp4
