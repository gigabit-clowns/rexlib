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
 * @file cuda_memory_block_context.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_memory_block_context.hpp
 * @date 2024-11-06
 * 
 */

#include "cuda_memory_block_context.hpp"

namespace xmipp4
{
namespace compute
{

template <typename Iterator>
inline
cuda_memory_block_context<Iterator>
::cuda_memory_block_context(iterator prev, 
                            iterator next, 
                            bool free ) noexcept
    : m_prev(prev)
    , m_next(next)
    , m_free(free)
{
}

template <typename Iterator>
inline
void cuda_memory_block_context<Iterator>
::set_previous_block(iterator prev) noexcept
{
    m_prev = prev;
}

template <typename Iterator>
inline
typename cuda_memory_block_context<Iterator>::iterator
cuda_memory_block_context<Iterator>::get_previous_block() const noexcept
{
    return m_prev;
}

template <typename Iterator>
inline
void cuda_memory_block_context<Iterator>
::set_next_block(iterator next) noexcept
{
    m_next = next;
}

template <typename Iterator>
inline
typename cuda_memory_block_context<Iterator>::iterator
cuda_memory_block_context<Iterator>::get_next_block() const noexcept
{
    return m_next;
}

template <typename Iterator>
inline
void cuda_memory_block_context<Iterator>::set_free(bool free)
{
    m_free = free;
}

template <typename Iterator>
inline
bool cuda_memory_block_context<Iterator>::is_free() const noexcept
{
    return m_free;
}

} // namespace compute
} // namespace xmipp4
