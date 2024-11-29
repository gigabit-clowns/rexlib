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
 * @file cuda_memory_block_cache.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_memory_block_cache.hpp
 * @date 2024-11-28
 * 
 */

#include "cuda_memory_allocator_delete.hpp"

namespace xmipp4
{
namespace compute
{

template <typename Allocator>
inline
cuda_memory_allocator_delete<Allocator>
::cuda_memory_allocator_delete(allocator_type& allocator) noexcept
    : m_allocator(allocator)
{
}

template <typename Allocator>
inline
void cuda_memory_allocator_delete<Allocator>
::operator()(const cuda_memory_block *block) const
{
    if (block)
    {
        m_allocator.get().deallocate(*block);
    }
}

} // namespace compute
} // namespace xmipp4
