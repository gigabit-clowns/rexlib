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
 * @file default_cuda_device_buffer.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines the compute::default_cuda_device_buffer class
 * @date 2024-10-30
 * 
 */

#include <xmipp4/cuda/compute/cuda_device_buffer.hpp>
#include <xmipp4/cuda/compute/allocator/cuda_memory_allocator_delete.hpp>

#include <memory>
#include <vector>

namespace xmipp4 
{
namespace compute
{

class cuda_memory_block;
class cuda_device_memory_allocator;
class cuda_device_queue;



class default_cuda_device_buffer final
    : public cuda_device_buffer
{
public:
    default_cuda_device_buffer(numerical_type type,
                               std::size_t count,
                               cuda_memory_block &block, 
                               cuda_device_memory_allocator &allocator ) noexcept;
    default_cuda_device_buffer(const default_cuda_device_buffer &other) = delete;
    default_cuda_device_buffer(default_cuda_device_buffer &&other) = default;
    ~default_cuda_device_buffer() override = default;

    default_cuda_device_buffer& 
    operator=(const default_cuda_device_buffer &other) = delete;
    default_cuda_device_buffer& 
    operator=(default_cuda_device_buffer &&other) = default;


    numerical_type get_type() const noexcept override;
    std::size_t get_count() const noexcept override;

    void* get_data() noexcept override;
    const void* get_data() const noexcept override;

    host_buffer* get_host_accessible_alias() noexcept override;
    const host_buffer* get_host_accessible_alias() const noexcept override;

    void record_queue(device_queue &queue) override;
    void record_queue_impl(cuda_device_queue &queue);

private:
    using block_delete = 
        cuda_memory_allocator_delete<cuda_device_memory_allocator>;

    numerical_type m_type;
    std::size_t m_count;
    std::unique_ptr<cuda_memory_block, block_delete> m_block;
}; 

} // namespace compute
} // namespace xmipp4
