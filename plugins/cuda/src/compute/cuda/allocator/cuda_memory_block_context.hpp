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
 * @file cuda_memory_block_context.hpp
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

template <typename Iterator>
class cuda_memory_block_context
{
public:
    using iterator = Iterator;

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
    
} // namespace compute
} // namespace xmipp4

#include "cuda_memory_block_context.inl"
