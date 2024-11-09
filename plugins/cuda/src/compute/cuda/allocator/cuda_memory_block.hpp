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
 * @file cuda_memory_block.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines data structure representing a memory block.
 * @date 2024-11-06
 * 
 */

#include <cstddef>

namespace xmipp4 
{
namespace compute
{

class cuda_memory_block
{
public:
    cuda_memory_block(void *data, 
                      std::size_t size, 
                      std::size_t queue_id ) noexcept;
    cuda_memory_block(const cuda_memory_block &other) = default;
    cuda_memory_block(cuda_memory_block &&other) = default;
    ~cuda_memory_block() = default;

    cuda_memory_block& operator=(const cuda_memory_block &other) = default;
    cuda_memory_block& operator=(cuda_memory_block &&other) = default;

    void* get_data() const noexcept;
    std::size_t get_size() const noexcept;
    std::size_t get_queue_id() const noexcept;

private:
    void *m_data;
    std::size_t m_size;
    std::size_t m_queue_id;
        
}; 

struct cuda_memory_block_size_less
{
    bool operator()(const cuda_memory_block &lhs, 
                    const cuda_memory_block &rhs ) const noexcept;
};

} // namespace compute
} // namespace xmipp4

#include "cuda_memory_block.inl"
