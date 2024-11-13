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
 * @file cuda_host_memory_allocator.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines the compute::cuda_host_memory_allocator interface
 * @date 2024-11-06
 * 
 */

#include "allocator/cuda_host_malloc.hpp"
#include "allocator/cuda_memory_cache.hpp"

#include <xmipp4/core/compute/host_memory_allocator.hpp>
#include <xmipp4/core/platform/attributes.hpp>

#include <mutex>

namespace xmipp4 
{
namespace compute
{

class cuda_host_memory_allocator
    : public host_memory_allocator
{
public:
    cuda_host_memory_allocator() = default;
    cuda_host_memory_allocator(const cuda_host_memory_allocator &other) = delete;
    cuda_host_memory_allocator(cuda_host_memory_allocator &&other) = delete;
    ~cuda_host_memory_allocator() override = default;

    cuda_host_memory_allocator&
    operator=(const cuda_host_memory_allocator &other) = delete;
    cuda_host_memory_allocator&
    operator=(cuda_host_memory_allocator &&other) = delete;

    std::unique_ptr<host_buffer> 
    create_buffer(numerical_type type, 
                  std::size_t count ) override;

    std::shared_ptr<host_buffer> 
    create_buffer_shared(numerical_type type, 
                         std::size_t count ) override;
    
    const cuda_memory_block& allocate(numerical_type type, std::size_t count);
    void deallocate(const cuda_memory_block &block);

private:
    XMIPP4_NO_UNIQUE_ADDRESS cuda_host_malloc m_allocator;
    cuda_memory_cache m_cache;
    std::mutex m_mutex;

}; 

} // namespace compute
} // namespace xmipp4
